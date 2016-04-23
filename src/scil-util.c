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

size_t scilU_write_dims_to_buffer(void* dest, const scil_dims* dims){

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

void scilU_read_dims_from_buffer(scil_dims* dims, void* dest){
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
}


static unsigned char sig_bits[MANTISSA_MAX_LENGTH_P1] = {255};
static unsigned char sig_decimals[MANTISSA_MAX_LENGTH_P1] = {255};

#define LOG10B2 3.3219280948873626
#define LOG2B10 0.30102999566398114

static void compute_significant_bit_mapping(){

	if(sig_bits[0] != 255) return;

	for(int i = 0; i < MANTISSA_MAX_LENGTH_P1; ++i){
		sig_bits[i] = (unsigned char)ceil(i * LOG10B2);
		sig_decimals[i] = (unsigned char)floor(i * LOG2B10);
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
  if(bits == 0){
		return 0;
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
