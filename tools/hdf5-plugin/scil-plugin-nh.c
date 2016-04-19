// This file is part of SCIL.
//
// SCIL is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SCIL is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with SCIL.  If not, see <http://www.gnu.org/licenses/>.

// Code is partially based on MAFISC filter

#include <hdf5.h>

#include <scil.h>

#define H5_HAVE_FILTER_SCIL
#define SCIL_ID 32003

H5PL_type_t H5PLget_plugin_type(void) {
	return H5PL_TYPE_FILTER;
}

static htri_t compressorCanCompress(hid_t dcpl_id, hid_t type_id, hid_t space_id);
static herr_t compressorSetLocal(hid_t pList, hid_t type, hid_t space);
static size_t compressorFilter(unsigned int flags, size_t cd_nelmts, const unsigned int cd_values[], size_t nBytes, size_t *buf_size, void **buf);

const void *H5PLget_plugin_info(void) {
	static H5Z_class2_t filterClass = {
		.version = H5Z_CLASS_T_VERS,
		.id = SCIL_ID,
		.encoder_present = 1,
		.decoder_present = 1,
		.name = "SCIL",
		.can_apply = & compressorCanCompress,
		.set_local = & compressorSetLocal,
		.filter = & compressorFilter
	};
	return &filterClass;
}

static htri_t compressorCanCompress(hid_t dcpl_id, hid_t type_id, hid_t space_id) {
	htri_t result = H5Sis_simple(space_id);
	H5T_class_t dataTypeClass;
	if(result <= 0) return result;	//The dataspace must be simple.
	dataTypeClass = H5Tget_class(type_id);
	switch(dataTypeClass) {
		case H5T_INTEGER:
		case H5T_FLOAT:
			switch(H5Tget_size(type_id)) {
				case 1:
				case 2:
				case 4:
				case 8: return 1;
				default: return 0;
			}
		case H5T_NO_CLASS:
		case H5T_TIME:
		case H5T_STRING:
		case H5T_BITFIELD:
		case H5T_OPAQUE:
		case H5T_COMPOUND:
		case H5T_REFERENCE:
		case H5T_ENUM:
		case H5T_VLEN:
		case H5T_ARRAY:
		case H5T_NCLASSES:	//default would have been sufficient...
		default: return 0;
	}
	dcpl_id = 0;	//Shut up. Never executed.
}

static herr_t compressorSetLocal(hid_t pList, hid_t type, hid_t space) {
//	fprintf(stderr, "compressorSetLocal()\n");
	int rank = H5Sget_simple_extent_ndims(space), i;
	if(rank <= 0) return -4;
	{
		hsize_t chunkSize[rank];	//Once again an unfounded compiler complaint.
		int chunkRank = H5Pget_chunk(pList, rank, chunkSize);
		if(chunkRank <= 0) return -1;
		if(chunkRank > rank) return -2;
		{
			unsigned int cd_values[sizeof(CompressorPrivate)/sizeof(unsigned int) + (size_t)chunkRank];	//yaucc = Yet Another Unfounded Compiler Complaint.
			CompressorPrivate* privates = (CompressorPrivate*)cd_values;
			privates->version = 0;
			privates->datasetId = (unsigned int)((rand() << 16) ^ rand());	//Two calls to make sure all bits are random, otherwise the msb would be zero...
			if(!(privates->dataTypeSize = (unsigned int)H5Tget_size(type))) return -5;
			privates->isFloat = (H5Tget_class(type) == H5T_FLOAT);
			switch(H5Tget_order(type)) {
				case H5T_ORDER_LE:
					privates->byteOrderIsLE = 1;
					break;
				case H5T_ORDER_BE:
					privates->byteOrderIsLE = 0;
					break;
				case H5T_ORDER_ERROR:
				case H5T_ORDER_VAX:
				case H5T_ORDER_MIXED:
				case H5T_ORDER_NONE:
				default:
					return -3;	//Don't know about VAX-Order...
			}
			privates->rank = (unsigned int)chunkRank;
			for(i = 0; i < chunkRank; i++) {
				if(chunkSize[i] > 0xffffffffl) return -6;	//Safety.
				privates->dimSizes[i] = (unsigned int)(chunkSize[i]);
			}
			return H5Pmodify_filter(pList, SCIL_ID,	H5Z_FLAG_OPTIONAL, sizeof(CompressorPrivate)/sizeof(unsigned int) + (unsigned int)chunkRank, cd_values );
		}
	}
}

static size_t compressorFilter(unsigned int flags, size_t cd_nelmts, const unsigned int cd_values[], size_t nBytes, size_t *buf_size, void **buf) {
	clock_t startTime = clock(), opStart;
	long i;
	const CompressorPrivate* privates = (const CompressorPrivate*)cd_values;
	//Get the native byte order.
	int64_t test64 = 0x00ffffffffffff01;
	int32_t test32 = 0x00ffff01;
	int16_t test16 = 0x0001;
	long nativeIsLE = *(char*)&test16;
	if(*(char*)&test64 != *(char*)&test32 || *(char*)&test32 != *(char*)&test16) return 0;	//Fail, if we got a mixed native byte order.
	//Check for decompression mode.
	if(flags & H5Z_FLAG_REVERSE) return decompress(nBytes, buf_size, buf);
	//Read the cd_values, doing some thorough error checking.
	if(cd_nelmts < 1) return 0;	//Fail, if no metadata is handed in.
	if(cd_values[0] != 0) return 0;	//Fail, if the metadata format is not known.
	if(cd_nelmts < sizeof(CompressorPrivate)/sizeof(unsigned int)) return 0;	//Fail, if there is not enough metadata.
	if(cd_nelmts != sizeof(CompressorPrivate)/sizeof(unsigned int) + privates->rank) return 0;	//Fail, if the dimension count is wrong.
	{
		unsigned long chunkSize = 1;
		size_t sizeTDims[privates->rank], curDataSize, bestDataSize, cd_valuesSize = cd_nelmts*sizeof(unsigned int), result, bestMethodStringSize;	//yaucc
		void* buffer1, *buffer2, *curBuffer, *bestBuffer;
		char* bestMethodString;
		for(i = 0; i < privates->rank; i++) {
			chunkSize *= privates->dimSizes[i];
			sizeTDims[i] = privates->dimSizes[i];
		}
		if(nBytes != chunkSize*privates->dataTypeSize) return 0;	//Refuse to work on anything but a complete chunk!
		//Set up the filters and the buffers.
		bestMethodString = getCompressionMethodString(privates->datasetId);
		buffer1 = malloc(nBytes);
		buffer2 = malloc(nBytes);
		curBuffer = malloc(nBytes);
		bestBuffer = malloc(nBytes);
		//Convert to 2-complement integers in native byte order.
		if((privates->byteOrderIsLE && !nativeIsLE) || (!privates->byteOrderIsLE && nativeIsLE)) {
			if(privates->isFloat) {
				switchByteOrder(privates->dataTypeSize, chunkSize, *buf, buffer2);
				doFpXpConversion(privates->dataTypeSize, privates->rank, sizeTDims, buffer2, buffer1);
			} else {
				switchByteOrder(privates->dataTypeSize, chunkSize, *buf, buffer1);
			}
		} else {
			if(privates->isFloat) {
				doFpXpConversion(privates->dataTypeSize, privates->rank, sizeTDims, *buf, buffer1);
			} else {
				memmove(buffer1, *buf, nBytes);
			}
    }
  }
}
