#include <scil-dims.h>

#include <scil-util.h>

#include <assert.h>
#include <string.h>

void scil_init_dims_1d(scil_dims* dims, size_t dim1)
{
    dims->dims      = 1;
    dims->length[0] = dim1;
}

void scil_init_dims_2d(scil_dims* dims, size_t dim1, size_t dim2)
{
    dims->dims      = 2;
    dims->length[0] = dim1;
    dims->length[1] = dim2;
}

void scil_init_dims_3d(scil_dims* dims, size_t dim1, size_t dim2, size_t dim3)
{
	dims->dims = 3;
	dims->length[0] = dim1;
	dims->length[1] = dim2;
	dims->length[2] = dim3;
}

void scil_init_dims_4d(scil_dims* dims, size_t dim1, size_t dim2, size_t dim3, size_t dim4)
{
	dims->dims = 4;
	dims->length[0] = dim1;
	dims->length[1] = dim2;
	dims->length[2] = dim3;
	dims->length[3] = dim4;
}

void scil_copy_dims_array(scil_dims* out_dims, scil_dims in_dims)
{
	out_dims->dims = in_dims.dims;
	memcpy(out_dims->length, & in_dims.length, sizeof(size_t)* in_dims.dims);
}

void scil_init_dims_array(scil_dims* dims, uint8_t count, const size_t* length)
{
    dims->dims = count;
    assert(count <= 4);
    memcpy(&dims->length, length, count * sizeof(size_t));
}

size_t scil_get_data_count(const scil_dims* dims)
{
    size_t result = 1;
    for (uint8_t i = 0; i < dims->dims; ++i) {
        result *= dims->length[i];
    }
    return result;
}

size_t scil_get_data_size(enum SCIL_Datatype datatype, const scil_dims* dims)
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

size_t scil_compress_buffer_size_bound(enum SCIL_Datatype datatype,
                                       const scil_dims* dims)
{
    return scil_get_data_size(datatype, dims) * 4 + SCIL_BLOCK_HEADER_MAX_SIZE;
}
