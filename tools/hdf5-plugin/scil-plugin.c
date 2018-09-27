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

// @see: https://www.hdfgroup.org/HDF5/doc/Advanced/DynamicallyLoadedFilters/HDF5DynamicallyLoadedFilters.pdf

#include <assert.h>
#include <hdf5.h>
#include <string.h>

#include <scil-hdf5-plugin.h>

#include <scil.h>
#include <scil-util.h>

#ifdef DEBUG
#define debug(...) printf("[SCIL HDF5] "__VA_ARGS__)
#else
#define debug(...)
#endif

#define error(...) printf("[SCIL HDF5] ERROR: "__VA_ARGS__)

#define PLUGIN_ERROR 0
#define PLUGIN_OK 1

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
		case H5T_ENUM:
		case H5T_INTEGER:
			switch(H5Tget_size(type_id)) {
				case 1:
				case 2:
				case 4:
				case 8: return PLUGIN_OK;
				default: return PLUGIN_ERROR;
			}
		case H5T_FLOAT:
			switch(H5Tget_size(type_id)) {
				case 4:
				case 8: return PLUGIN_OK;
				default: return PLUGIN_ERROR;
			}
		case H5T_STRING:
			return PLUGIN_OK;
		case H5T_NO_CLASS:
		case H5T_TIME:
		case H5T_BITFIELD:
		case H5T_OPAQUE:
		case H5T_COMPOUND:
		case H5T_REFERENCE:
		case H5T_VLEN:
		case H5T_ARRAY:
		case H5T_NCLASSES:	//default would have been sufficient...
		default: return PLUGIN_ERROR;
	}
}

typedef struct {
	scil_context_t* ctx;
	size_t dst_size; // the size of the buffer
} plugin_compress_config;

typedef struct {
	plugin_compress_config * cfg;

	scil_dims_t dims;
	enum SCIL_Datatype type;
} plugin_config_persisted;

static herr_t compressorSetLocal(hid_t pList, hid_t type_id, hid_t space) {
	//debug("compressorSetLocal()\n");
	int rank = H5Sget_simple_extent_ndims(space);
	if(rank <= 0) return -4;

	/*if (rank > 4){
		error("SCIL only supports up to 4D variables\n");
		exit(1);
	}*/

	const int cd_size = sizeof(plugin_config_persisted) / sizeof(unsigned int);

	unsigned int cd_values[cd_size];
	memset(cd_values, 0, sizeof(cd_values));
	plugin_config_persisted * cfg_p = (plugin_config_persisted *) cd_values;

	cfg_p->cfg = (plugin_compress_config*) malloc(sizeof(plugin_compress_config));
	plugin_compress_config * config = cfg_p->cfg;

  hsize_t chunkSize[rank];
	int chunkRank = H5Pget_chunk(pList, rank, chunkSize);
	if(chunkRank <= 0) return -1;
	if(chunkRank > rank) return -2;
	/*if(chunkRank > 4){
			for(int i=4; i<rank;i++){
				chunkSize[3] *= chunkSize[i];
			}
			rank=4;
	}*/

	assert(sizeof(size_t) == sizeof(hsize_t) );
	scil_dims_initialize_array(& cfg_p->dims, rank, (const size_t*) chunkSize);

  scil_user_hints_t * h;
	scil_user_hints_t hints_new;
	// TODO set the hints (accuracy) according to the property lists in HDF5
	// H5Tget_precision ?
	int ret = H5Pget_scil_user_hints_t(pList, & h);
	if(ret != 0 || h == NULL){
		h = & hints_new;
		scil_user_hints_initialize(h);
	}


	H5T_class_t dataTypeClass;
	dataTypeClass = H5Tget_class(type_id);

	switch(dataTypeClass) {
		case H5T_ENUM:
		case H5T_INTEGER:
			switch(H5Tget_size(type_id)) {
				case 1:
					cfg_p->type = SCIL_TYPE_INT8; break;
				case 2:
					cfg_p->type = SCIL_TYPE_INT16; break;
				case 4:
					cfg_p->type = SCIL_TYPE_INT32; break;
				case 8:
					cfg_p->type = SCIL_TYPE_INT64; break;
			}
			break;
		case H5T_FLOAT:
			switch(H5Tget_size(type_id)) {
				case 4: cfg_p->type = SCIL_TYPE_FLOAT; break;
				case 8: cfg_p->type = SCIL_TYPE_DOUBLE; break;
			}
			break;
		case H5T_STRING:
			cfg_p->type = SCIL_TYPE_STRING;
			break;
		default:
			assert(0);
	}

	herr_t hret;
	H5D_fill_value_t status;
	void * special_values = NULL;
	int special_cnt = 0;
	hret = H5Pfill_value_defined(pList, & status);
	if (hret >= 0 &&  status != H5D_FILL_VALUE_UNDEFINED){
		void * fill_value = malloc(1024); // TOOD find proper size
		hret = H5Pget_fill_value(pList, type_id, fill_value );
		if (hret >= 0){
			special_cnt = 1;
			special_values = fill_value;
		}
	}

  ret = scil_context_create(&config->ctx, cfg_p->type, special_cnt, special_values, h);
	if(special_values != NULL){
		free(special_values);
	}

	assert(ret == SCIL_NO_ERR);

	config->dst_size = scil_get_compressed_data_size_limit(& cfg_p->dims, cfg_p->type);

	// now we store the options with the dataset, this is actually not needed...
	return H5Pmodify_filter( pList, SCIL_ID, H5Z_FLAG_MANDATORY, cd_size, cd_values );
}

static size_t compressorFilter(unsigned int flags, size_t cd_nelmts, const unsigned int cd_values[], size_t nBytes, size_t *buf_size, void **buf){
	//debug("compressorFilter called %d %lld %lld %d \n", flags, (long long) nBytes, (long long) * buf_size, (int) cd_nelmts);

	size_t out_size = *buf_size;
	int ret;

  if(flags & H5Z_FLAG_REVERSE){
		// uncompress
		plugin_config_persisted* cfg_p = ((plugin_config_persisted *) cd_values);

		const size_t buff_size = scil_get_compressed_data_size_limit(& cfg_p->dims, cfg_p->type);
		byte * buffer = (byte*) malloc(buff_size);

		byte * in_buf = ((byte**) buf)[0];

		size_t c_buf_size;
		scilU_unpack8(in_buf, &c_buf_size);
		in_buf += 8;

		debug("DC: %zu \n", c_buf_size);

		ret = scil_decompress(cfg_p->type, buffer, & cfg_p->dims, in_buf, c_buf_size, buffer + buff_size/2 + 1);

		*buf = buffer;

	}else{ // compress

		plugin_config_persisted* cfg_p = ((plugin_config_persisted *) cd_values);
		plugin_compress_config* config = cfg_p->cfg;

		byte * buffer = (byte*) malloc(config->dst_size + 8);
		// memset(buffer, 0, config->dst_size + 8);

		ret = scil_compress(buffer + 8, config->dst_size, ((byte**) buf)[0], (& cfg_p->dims), & out_size, config->ctx);
		scilU_pack8(buffer, out_size);
		debug("ret: %zu \n", ret);
		debug("CS: %zu \n", out_size);
		out_size +=8;

		*buf = buffer;
	}
	assert(ret == SCIL_NO_ERR);

  return out_size; // 0 means error.
}

#define H5P_SCIL_HINT "scil_user_hints_t"


herr_t H5Pset_scil_user_hints_t(hid_t dcpl, scil_user_hints_t * hints){
	// TODO fixme, duplicate memory structure
	unsigned cd_values[2];
	scil_user_hints_t * hints_new = malloc(sizeof(scil_user_hints_t));
	memcpy(hints_new, hints, sizeof(scil_user_hints_t));
	memcpy(& cd_values[0], & hints_new, sizeof(void *));
	//printf("set %u %u \n", cd_values[0], cd_values[1]);
	//debug("H5Pset_scil_user_hints_t hints:%p\n", (void*) hints_new);

	return H5Pmodify_filter( dcpl, SCIL_ID, H5Z_FLAG_MANDATORY, 2, (unsigned*) cd_values );
}

herr_t H5Pget_scil_user_hints_t(hid_t dcpl, scil_user_hints_t ** out_hints){
	unsigned int *flags = NULL;
	size_t cd_nelmts = 2;

	unsigned cd_values[2];
	unsigned *filter_config = NULL;

	herr_t ret = H5Pget_filter_by_id( dcpl, SCIL_ID, flags, & cd_nelmts, cd_values, 0, NULL, filter_config);
	if(ret >= 0 && cd_nelmts == 2){
		//printf("get %u %u \n", cd_values[0], cd_values[1]);
		memcpy(out_hints, & cd_values[0], sizeof(void *));
		//printf("get %p \n", *out_hints);
		return 0;
	}
	return -1;
}
