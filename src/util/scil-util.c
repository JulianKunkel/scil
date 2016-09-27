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

#include <scil-util.h>

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>


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
	return (uint8_t)ceil(log2(100.0 / rel_tol));
}

double scilU_significant_bits_to_relative_tolerance(uint8_t sig_bits){
	return 100.0 / exp2(sig_bits);
}

struct timespec scilU_time_diff (struct timespec end, struct timespec start)
{
    struct timespec diff;
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

struct timespec scilU_time_sum (struct timespec t1, struct timespec t2)
{
    struct timespec sum;
    sum.tv_nsec = t1.tv_nsec + t2.tv_nsec;
    sum.tv_sec = sum.tv_nsec / 1000000000;
    sum.tv_nsec %= 1000000000;
    sum.tv_sec += t1.tv_sec + t2.tv_sec;
    return sum;
}

double scilU_time_to_double (struct timespec t)
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



void print_time (struct timespec time, FILE* file)
{
    fprintf(file, "%lu.%09lu", time.tv_sec, time.tv_nsec);
}

void scilU_critical_error(const char * msg){
  printf("Critical error: %s\n", msg);
  // printf("%d", 1/0);
  exit(1);
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
size_t scilG_data_pos(const scil_dims_t* pos, const scil_dims_t* size){
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


void scilG_iter(double* data,
                const scil_dims_t* dims,
                const scil_dims_t* offset,
                const scil_dims_t* ende,
                int* iter,
                scilG_iterfunc func,
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



static void scilU_plot1D(const char* name, scil_dims_t dims, double * buffer_in){
  FILE * f = fopen(name, "w");
  if(f == NULL){
    printf("Coult not open %s for write\n", name);
    exit(1);
  }
  fprintf(f, "%zu\n", dims.length[0]);
  fprintf(f, "%f", buffer_in[0]);
  for(size_t x = 1; x < dims.length[0]; x++){
    fprintf(f, ",%f", buffer_in[x]);
  }
  fprintf(f, "\n");
  fclose(f);
}

static void scilU_plot2D(const char* name, scil_dims_t dims, double * buffer_in){
  FILE * f = fopen(name, "w");
  if(f == NULL){
    printf("Coult not open %s for write\n", name);
    exit(1);
  }
  fprintf(f, "%zu, %zu\n", dims.length[0], dims.length[1]);
  for(size_t y = 0; y < dims.length[1]; y++){
    fprintf(f, "%f", buffer_in[0+ y * dims.length[0]]);
    for(size_t x = 1; x < dims.length[0]; x++){
      fprintf(f, ",%f", buffer_in[x+ y * dims.length[0]]);
    }
    fprintf(f, "\n");
  }
  fclose(f);
}

static void scilU_plot3D(const char* name, scil_dims_t dims, double * buffer_in){
  FILE * f = fopen(name, "w");
  if(f == NULL){
    printf("Coult not open %s for write\n", name);
    exit(1);
  }
  fprintf(f, "%zu, %zu, %zu\n", dims.length[0], dims.length[1], dims.length[2]);

  for(size_t z = 0; z < dims.length[2]; z++){
    size_t z_ = z * dims.length[0]*dims.length[1];
    for(size_t y = 0; y < dims.length[1]; y++){
      fprintf(f, "%f", buffer_in[0+ y * dims.length[0] + z_]);
      for(size_t x = 1; x < dims.length[0]; x++){
        fprintf(f, ",%f", buffer_in[x+ y * dims.length[0] + z_]);
      }
      fprintf(f, "\n");
    }
  }
  fclose(f);
}

void scilU_plot(const char* name, scil_dims_t dims, double * buffer_in){
  switch(dims.dims){
    case (1):{
       scilU_plot1D(name, dims, buffer_in);
       break;
     }case (2):{
       scilU_plot2D(name, dims, buffer_in);
       break;
     }case (3):{
       scilU_plot3D(name, dims, buffer_in);
       break;
     }default:{
       printf("Error can only plot up to 3D\n");
       exit(1);
     }
   }
 }
