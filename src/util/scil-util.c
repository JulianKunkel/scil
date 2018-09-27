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

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <float.h>


#include <scil-user-hints.h>
#include <scil-util.h>

void * scilU_safe_malloc(size_t size){
  void * p = malloc(size);
  if( p == NULL ){
    printf("[SCIL] Could not allocate %llu bytes\n", (unsigned long long) size);
    exit(1);
  }
  return p;
}

void scilU_find_minimum_maximum_with_excluded_points(SCIL_Datatype_t datatype, byte * data, scil_dims_t * dims, double * out_min, double * out_max, double ignore_to, double ignore_from, double fill_value){
  size_t count = scil_dims_get_count(dims);

  switch(datatype){
  case (SCIL_TYPE_DOUBLE): {
    scilU_find_minimum_maximum_with_excluded_points_double((double*) data, count, out_min, out_max, ignore_to, ignore_from, fill_value);
    return;
  }
  case (SCIL_TYPE_FLOAT) : {
    float min, max;
    scilU_find_minimum_maximum_with_excluded_points_float((float*) data, count, & min, & max, ignore_to, ignore_from, fill_value);
    *out_min = (double) min;
    *out_max = (double) max;
    return;
  }
  case(SCIL_TYPE_INT8):{
    int8_t min, max;
    scilU_find_minimum_maximum_with_excluded_points_int8_t((int8_t*)data, count, & min, & max, ignore_to, ignore_from, fill_value);
    *out_min = (double) min;
    *out_max = (double) max;
    return;
  }
  case(SCIL_TYPE_INT16):{
    int16_t min, max;
    scilU_find_minimum_maximum_with_excluded_points_int16_t((int16_t*)data, count, & min, & max, ignore_to, ignore_from, fill_value);
    *out_min = (double) min;
    *out_max = (double) max;
    return;
  }
  case(SCIL_TYPE_INT32):{
    int32_t min, max;
    scilU_find_minimum_maximum_with_excluded_points_int32_t((int32_t*)data, count, & min, & max, ignore_to, ignore_from, fill_value);
    *out_min = (double) min;
    *out_max = (double) max;
    return;
  }
  case(SCIL_TYPE_INT64):{
    int64_t min, max;
    scilU_find_minimum_maximum_with_excluded_points_int64_t((int64_t*) data, count, & min, & max, ignore_to, ignore_from, fill_value);
    *out_min = (double) min;
    *out_max = (double) max;
    return;
  }
  case(SCIL_TYPE_UNKNOWN) :
  case(SCIL_TYPE_BINARY):
  case(SCIL_TYPE_STRING):{
    assert(0 && "unsupported min/max");
  }
  }
}

void scilU_find_minimum_maximum(SCIL_Datatype_t datatype, byte * data, scil_dims_t * dims, double * out_min, double * out_max){
  scilU_find_minimum_maximum_with_excluded_points(datatype, data, dims, out_min, out_max, -DBL_MAX, DBL_MAX, DBL_MAX);
}

void scilU_subtract_data(SCIL_Datatype_t datatype, byte * restrict  data1, byte * restrict in_out_data2, scil_dims_t * dims){
  size_t count = scil_dims_get_count(dims);

  switch(datatype){
  case (SCIL_TYPE_DOUBLE): {
    scilU_subtract_data_double((double*) data1, (double*) in_out_data2, count);
    return;
  }
  case (SCIL_TYPE_FLOAT) : {
    scilU_subtract_data_float((float*) data1, (float*) in_out_data2, count);
    return;
  }
  case(SCIL_TYPE_INT8):{
    scilU_subtract_data_int8_t((int8_t*)data1,  (int8_t*) in_out_data2, count);
    return;
  }
  case(SCIL_TYPE_INT16):{
    scilU_subtract_data_int16_t((int16_t*)data1,  (int16_t*) in_out_data2, count);
    return;
  }
  case(SCIL_TYPE_INT32):{
    scilU_subtract_data_int32_t((int32_t*)data1,  (int32_t*) in_out_data2, count);
    return;
  }
  case(SCIL_TYPE_INT64):{
    scilU_subtract_data_int64_t((int64_t*) data1, (int64_t*) in_out_data2, count);
    return;
  }
  case(SCIL_TYPE_UNKNOWN) :
  case(SCIL_TYPE_BINARY):
  case(SCIL_TYPE_STRING):{
    assert(0 && "unsupported");
  }
  }
}


void* safe_malloc (size_t size, const char* file, unsigned long line)
{
    assert (size != 0);

    void* p = malloc (size);
    if (p == NULL)
    {
        fprintf (stderr, "[%s:%lu] malloc failed (%lu bytes)\n", file, line,
                 size);
        exit (EXIT_FAILURE);
    }
    return p;
}


void* safe_calloc (size_t nmemb, size_t size, const char* file,
                   unsigned long line)
{
    assert (nmemb != 0);
    assert (size != 0);

    void* p = calloc (nmemb, size);
    if (p == NULL)
    {
        fprintf (stderr, "[%s:%lu] calloc failed (%lu bytes)\n", file, line,
                 nmemb * size);
        exit (EXIT_FAILURE);
    }
    return p;
}


void* safe_realloc (void* ptr, size_t size, const char* file,
                    unsigned long line)
{
    assert (ptr != NULL);
    assert (size != 0);

    void* p = realloc (ptr, size);
    if (p == NULL)
    {
        fprintf (stderr, "[%s:%lu] realloc failed (%lu bytes)\n", file, line,
                 size);
        exit (EXIT_FAILURE);
    }
    return p;
}

FILE* safe_fopen(const char* path, const char* args, const char* src, unsigned long line){

    assert(path!=NULL);
    assert(args!=NULL);

    FILE* file = fopen(path, args);
    if(file == 0){
        fprintf (stderr, "[%s:%lu] fopen failed (path: %s, args: %s)\n", src, line, path, args);
        exit(EXIT_FAILURE);
    }

    return file;
}

size_t scilU_write_dims_to_buffer(void* dest, const scil_dims_t* dims){

    assert(dest != NULL);

    size_t header_size = 0;

    *((uint8_t*)dest) = dims->dims;
    dest = (char*)dest + 1;
    header_size++;

    for(uint8_t i = 0; i < dims->dims; ++i){
        *((size_t*)dest) = dims->length[i];
        dest = (char*)dest + 8;
        header_size += 8;
    }

    return header_size;
}

void scilU_read_dims_from_buffer(scil_dims_t* dims, void* dest){
    dims->dims = *((uint8_t*)dest);
    dest = (char*)dest + 1;

    for(uint8_t i = 0; i < dims->dims; ++i){
        dims->length[i] = *((uint64_t*)dest);
        dest = (char*)dest + 8;
    }
}

void scilU_print_buffer(char * dest, size_t out_size){
	for (size_t i=0; i < out_size ; i++){
		printf("%x", dest[i]);
	}
	printf("\n");
  //for (size_t i=0; i < out_size ; i++){
	//	printf("%c ", dest[i]);
	//}
	//printf("\n");
}


static unsigned char sig_bits[MANTISSA_MAX_LENGTH_P1] = {255};
static unsigned char sig_decimals[MANTISSA_MAX_LENGTH_P1] = {255};

#define LOG10B2 3.3219280948873626
#define LOG2B10 0.30102999566398114

static void compute_significant_bit_mapping(){

	if(sig_bits[0] != 255) return;

	for(int i = 0; i < MANTISSA_MAX_LENGTH_P1; ++i){
		sig_bits[i] = (unsigned char)ceil(i * LOG10B2);
		sig_decimals[i] = (unsigned char)ceil(i * LOG2B10);
	}
}

int scilU_convert_significant_decimals_to_bits(int decimals){
  if (decimals == SCIL_ACCURACY_INT_FINEST){
    return SCIL_ACCURACY_INT_FINEST;
  }
	compute_significant_bit_mapping();
	return sig_bits[decimals];
}

int scilU_convert_significant_bits_to_decimals(int bits){
    if (bits == SCIL_ACCURACY_INT_FINEST){
        return SCIL_ACCURACY_INT_FINEST;
    }
	// compute mapping between decimals and bits
	compute_significant_bit_mapping();
	return sig_decimals[bits];
}


uint8_t scilU_relative_tolerance_to_significant_bits(double rel_tol){
  // Result of this function can not be negative as for the used return type,
  // but the formula in theory could be. This would mean we could even
  // compress the exponent, but not implemented yet.
  // Also 1 sigbits means 0 mantissa bits, hence minimal return value.
  if (rel_tol > 50.0)
      return 1;
	return (uint8_t)ceil(log2(100.0 / rel_tol));
}

double scilU_significant_bits_to_relative_tolerance(uint8_t sig_bits){
	return 100.0 / exp2(sig_bits);
}

scil_timer scilU_time_diff (scil_timer end, scil_timer start){
    scil_timer diff;
    if (end.tv_nsec < start.tv_nsec)
    {
        diff.tv_sec = end.tv_sec - start.tv_sec - 1;
        diff.tv_nsec = 1000000000 + end.tv_nsec - start.tv_nsec;
    }
    else
    {
        diff.tv_sec = end.tv_sec - start.tv_sec;
        diff.tv_nsec = end.tv_nsec - start.tv_nsec;
    }
    return diff;
}

scil_timer scilU_time_sum (scil_timer t1, scil_timer t2)
{
    scil_timer sum;
    sum.tv_nsec = t1.tv_nsec + t2.tv_nsec;
    sum.tv_sec = sum.tv_nsec / 1000000000;
    sum.tv_nsec %= 1000000000;
    sum.tv_sec += t1.tv_sec + t2.tv_sec;
    return sum;
}

double scilU_time_to_double (scil_timer t)
{
    double d = (double)t.tv_nsec;
    d /= 1000000000.0;
    d += (double)t.tv_sec;
    return d;
}

void scilU_start_timer(scil_timer * t1){
  clock_gettime(CLOCK_MONOTONIC, t1);
}

double scilU_stop_timer(scil_timer t1){
  scil_timer end;
  scilU_start_timer(& end);
  return scilU_time_to_double(scilU_time_diff(end, t1));
}



void print_time (scil_timer time, FILE* file)
{
    fprintf(file, "%lu.%09lu", time.tv_sec, time.tv_nsec);
}

int scilU_double_equal(double val1, double val2){
  if ( val1 > val2){
    return val1 - val2 < 1e-300;
  }else{
    return val2 - val1 < 1e-300;
  }
}

int scilU_float_equal(float val1, float val2){
  if ( val1 > val2){
    return val1 - val2 < 1e-40f;
  }else{
    return val2 - val1 < 1e-40f;
  }
}


/*
\brief Convert the current position in a ND array to the position of the original 1D data array.
 */
size_t scilU_data_pos(const scil_dims_t* pos, const scil_dims_t* size){
  assert(size->dims == pos->dims);
  size_t cur = pos->length[size->dims - 1];
  if(size->dims == 0){
    return cur;
  }
  for(int i=size->dims - 2; i >= 0; i--){
    cur *= size->length[i+1];
    cur += pos->length[i];
  }
  return cur;
}


void scilU_iter(double* data,
                const scil_dims_t* dims,
                const scil_dims_t* offset,
                const scil_dims_t* ende,
                int* iter,
                scilU_iterfunc func,
                const void* user_ptr)
{
  assert(dims->dims > 0);
  // check arguments further
  for(int i = 0; i < dims->dims; i++){
    assert(dims->length[i] > 0);
    assert(ende->length[i] <= dims->length[i]);
  }

  scil_dims_t pos_dims;
  pos_dims.dims = dims->dims;
  size_t* pos = pos_dims.length;
  for(int i = 0; i < dims->dims; i++){
    pos[i] = offset->length[i];
  }
  int iter_local[dims->dims];

  if(iter == NULL){
    iter = iter_local;
    for(int i = 0; i < dims->dims; i++){
      iter[i] = 1;
    }
  }

  // descent into starting point
  int stackpos = dims->dims - 1;
  while(stackpos != -1){
    // walk over last dimension
    {
      size_t start = offset->length[dims->dims - 1];
      size_t end = ende->length[dims->dims - 1];
      for(size_t i=start; i < end; i+= iter[stackpos]){
        pos[stackpos] = i;
        func(data, &pos_dims, dims, iter, user_ptr);
      }
    }
    // propagate upwards
    stackpos--;
    while(stackpos != -1){
      pos[stackpos]+= iter[stackpos];

      size_t start = offset->length[stackpos];
      size_t end = ende->length[stackpos];

      if(pos[stackpos] < end){
        stackpos = dims->dims - 1;
        break;
      }
      pos[stackpos] = start;
      stackpos--;
    }
  }
}

void scilU_print_dims(scil_dims_t pos){
  printf("(");
  printf("%zu", pos.length[0]);
  for(int i=1; i < pos.dims; i++){
    printf(",%zu", pos.length[i]);
  }
  printf(") ");
}


size_t scil_dims_get_size(const scil_dims_t* dims, enum SCIL_Datatype datatype)
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

size_t scil_get_compressed_data_size_limit(const scil_dims_t* dims, enum SCIL_Datatype datatype)
{
    return scil_dims_get_size(dims, datatype) * 4 + SCIL_BLOCK_HEADER_MAX_SIZE;
}
