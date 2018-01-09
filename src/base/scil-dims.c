#include <scil-dims.h>

#include <scil-util.h>

#include <assert.h>
#include <string.h>

void scil_dims_initialize_1d(scil_dims_t* dims, size_t dim1)
{
  memset(dims, 0, sizeof(scil_dims_t));
  dims->dims      = 1;
  dims->length[0] = dim1;
}

void scil_dims_initialize_2d(scil_dims_t* dims, size_t dim1, size_t dim2)
{
  memset(dims, 0, sizeof(scil_dims_t));
  dims->dims      = 2;
  dims->length[0] = dim1;
  dims->length[1] = dim2;
}

void scil_dims_initialize_3d(scil_dims_t* dims, size_t dim1, size_t dim2, size_t dim3)
{
  memset(dims, 0, sizeof(scil_dims_t));
	dims->dims = 3;
	dims->length[0] = dim1;
	dims->length[1] = dim2;
	dims->length[2] = dim3;
}

void scil_dims_initialize_4d(scil_dims_t* dims, size_t dim1, size_t dim2, size_t dim3, size_t dim4)
{
	dims->dims = 4;
	dims->length[0] = dim1;
	dims->length[1] = dim2;
	dims->length[2] = dim3;
	dims->length[3] = dim4;
  if(dim4 == 0){
    dims->dims--;
    if(dim3 == 0){
      dims->dims--;
      if(dim2 == 0){
        dims->dims--;
      }
    }
  }
}

void scil_dims_initialize_5d(scil_dims_t* dims, size_t dim1, size_t dim2, size_t dim3, size_t dim4, size_t dim5)
{
	dims->dims = 5;
	dims->length[0] = dim1;
	dims->length[1] = dim2;
	dims->length[2] = dim3;
	dims->length[3] = dim4;
  dims->length[4] = dim5;
  if(dim5 == 0){
    dims->dims--;
    if(dim4 == 0){
      dims->dims--;
      if(dim3 == 0){
        dims->dims--;
        if(dim2 == 0){
          dims->dims--;
        }
      }
    }
  }
}

void scil_dims_initialize_array(scil_dims_t* dims, uint8_t count, const size_t* length)
{
  memset(dims, 0, sizeof(scil_dims_t));
  dims->dims = count;
  //assert(count <= 4);
  memcpy(&dims->length, length, count * sizeof(size_t));
}

void scil_dims_copy(scil_dims_t* out_dims, const scil_dims_t* in_dims)
{
	out_dims->dims = in_dims->dims;
	memcpy(out_dims->length, &in_dims->length, in_dims->dims * sizeof(size_t));
}

size_t scil_dims_get_count(const scil_dims_t* dims)
{
    size_t result = 1;
    for (uint8_t i = 0; i < dims->dims; ++i) {
        result *= dims->length[i];
    }
    return result;
}
