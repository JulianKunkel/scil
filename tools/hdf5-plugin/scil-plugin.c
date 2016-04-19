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

#include <scil-hdf5-plugin.h>

#include <scil.h>

#define DEBUG

#ifdef DEBUG
#define debug(...) printf(__VA_ARGS__)
#else
#define debug(...)
#endif

#pragma GCC diagnostic ignored "-Wunused-parameter"

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

  hsize_t chunkSize[rank];	//Once again an unfounded compiler complaint.
	int chunkRank = H5Pget_chunk(pList, rank, chunkSize);
	if(chunkRank <= 0) return -1;
	if(chunkRank > rank) return -2;

  debug("compressorSetLocal called\n");

  return 0;
}

static size_t compressorFilter(unsigned int flags, size_t cd_nelmts, const unsigned int cd_values[], size_t nBytes, size_t *buf_size, void **buf) {
  //if(flags & H5Z_FLAG_REVERSE) ;
  uint8_t v =  scil_compressor_num_by_name("memcpy");
  debug("compressorFilter called\n");
  return (size_t) v;
}
