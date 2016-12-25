#include <scil-dims.h>

#include <scil-util.h>

#include <assert.h>
#include <string.h>

void scilPr_initialize_dims_1d(scil_dims_t* dims, size_t dim1)
{
  memset(dims, 0, sizeof(scil_dims_t));
  dims->dims      = 1;
  dims->length[0] = dim1;
}

void scilPr_initialize_dims_2d(scil_dims_t* dims, size_t dim1, size_t dim2)
{
  memset(dims, 0, sizeof(scil_dims_t));
  dims->dims      = 2;
  dims->length[0] = dim1;
  dims->length[1] = dim2;
}

void scilPr_initialize_dims_3d(scil_dims_t* dims, size_t dim1, size_t dim2, size_t dim3)
{
  memset(dims, 0, sizeof(scil_dims_t));
	dims->dims = 3;
	dims->length[0] = dim1;
	dims->length[1] = dim2;
	dims->length[2] = dim3;
}

void scilPr_initialize_dims_4d(scil_dims_t* dims, size_t dim1, size_t dim2, size_t dim3, size_t dim4)
{
	dims->dims = 4;
	dims->length[0] = dim1;
	dims->length[1] = dim2;
	dims->length[2] = dim3;
	dims->length[3] = dim4;
}

void scilPr_initialize_dims_array(scil_dims_t* dims, uint8_t count, const size_t* length)
{
  memset(dims, 0, sizeof(scil_dims_t));
  dims->dims = count;
  assert(count <= 4);
  memcpy(&dims->length, length, count * sizeof(size_t));
}

void scilPr_copy_dims(scil_dims_t* out_dims, const scil_dims_t* in_dims)
{
	out_dims->dims = in_dims->dims;
	memcpy(out_dims->length, &in_dims->length, in_dims->dims * sizeof(size_t));
}

size_t scilPr_get_dims_count(const scil_dims_t* dims)
{
    size_t result = 1;
    for (uint8_t i = 0; i < dims->dims; ++i) {
        result *= dims->length[i];
    }
    return result;
}

size_t scilPr_get_dims_size(const scil_dims_t* dims, enum SCIL_Datatype datatype)
{
    if (dims->dims == 0) {
        return 0;
    }
    size_t result = 1;
    for (uint8_t i = 0; i < dims->dims; ++i) {
        result *= dims->length[i];
    }
    return result * DATATYPE_LENGTH(datatype);
}

size_t scilPr_get_compressed_data_size_limit(const scil_dims_t* dims, enum SCIL_Datatype datatype)
{
    return scilPr_get_dims_size(dims, datatype) * 4 + SCIL_BLOCK_HEADER_MAX_SIZE;
}
