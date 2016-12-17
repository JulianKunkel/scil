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

#ifndef SCIL_UTIL_H
#define SCIL_UTIL_H

/**
 * \file
 * \brief Contains miscellanious useful functions.
 * \author Julian Kunkel <juliankunkel@googlemail.com>
 * \author Lennart Braun <3braun@informatik.uni-hamburg.de>
 * \author Armin Schaare <3schaare@informatik.uni-hamburg.de>
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <scil.h>


/**
 * \brief Makro to make sure off_t is 64-bit unsigned integer
 */
#define _FILE_OFFSET_BITS 64

#define MANTISSA_MAX_LENGTH 52
#define MANTISSA_MAX_LENGTH_P1 65
#define MANTISSA_LENGTH_FLOAT 23
#define MANTISSA_LENGTH_DOUBLE 52

#define max(a,b) \
  (a > b ? a : b)

#define min(a,b) \
  (-max(-a, -b))

#define DATATYPE_LENGTH(type) (type == SCIL_TYPE_FLOAT ? sizeof(float) : type == SCIL_TYPE_DOUBLE ? sizeof(double) : type == SCIL_TYPE_INT8 ? sizeof(int8_t) : type == SCIL_TYPE_INT16 ? sizeof(int16_t) : type == SCIL_TYPE_INT32 ? sizeof(int32_t) : type == SCIL_TYPE_INT64 ? sizeof(int64_t) : type == SCIL_TYPE_BINARY ? 1 : 0)

/**
 * \brief Allocates a buffer with error checking.
 *
 * Uses malloc for allocation.
 *
 * \param size Length of requested memory in bytes.
 * \param file Filename to display in error message.
 * \param line Line to display in error message.
 * \pre size != 0
 * \return Pointer to allocated buffer.
 * \post Allocation successfull or exit with error message.
 */
void* safe_malloc (size_t size, const char* file, unsigned long line);


/**
 * \brief Allocates a buffer with error checking.
 *
 * Uses calloc for allocation.
 *
 * \param nmemb Number of elements.
 * \param size Length of one element.
 * \param file Filename to display in error message.
 * \param line Line to display in error message.
 * \pre nmemb != 0
 * \pre size != 0
 * \return Pointer to allocated buffer.
 * \post Allocation successfull or exit with error message.
 */
void* safe_calloc (size_t nmemb, size_t size, const char* file, unsigned long line);


/**
 * \brief Reallocates a buffer with error checking.
 *
 * Uses realloc for allocation.
 *
 * \param ptr  Pointer to the original buffer.
 * \param size Length of requested memory in bytes.
 * \param file Filename to display in error message.
 * \param line Line to display in error message.
 * \pre size != 0
 * \return Pointer to allocated buffer.
 * \post Allocation successfull or exit with error message.
 */
void* safe_realloc (void* ptr, size_t size, const char* file, unsigned long line);


/**
 * \brief Shortcut for safe_malloc().
 * \param size Length of requested memory in bytes.
 */
#define SAFE_MALLOC(size) safe_malloc(size, __FILE__, __LINE__)


/**
 * \brief Shortcut for safe_calloc().
 * \param nmemb Number of elements.
 * \param size Length of one element in bytes.
 */
#define SAFE_CALLOC(nmemb, size) safe_calloc(nmemb, size, __FILE__, __LINE__)


/**
 * \brief Shortcut for safe_realloc().
 * \param ptr  Pointer to an allocated buffer.
 * \param size Length of requested memory in bytes.
 */
#define SAFE_REALLOC(ptr, size) safe_realloc(ptr, size, __FILE__, __LINE__)

/**
 * \brief Opens a file with error checking.
 * \param path Path of the file to open.
 * \param args fopen argument string.
 * \param file Filename to display in error message.
 * \param line Line to display in error message.
 * \pre path != NULL
 * \pre args != NULL
 * \return Pointer to opened file.
 * \post Opened file successfully or exit with error message.
 */
FILE* safe_fopen(const char* path, const char* args, const char* src, unsigned long line);

/**
 * \brief Shortcut for safe_fopen().
 * \param path Path of the file to open.
 * \param args fopen argument string.
 */
#define SAFE_FOPEN(path, args) safe_fopen(path, args, __FILE__, __LINE__)



typedef union {
  struct {
    uint32_t mantissa  : MANTISSA_LENGTH_FLOAT;
    uint32_t exponent : 8;
    uint32_t sign     : 1;
  } p;
	float f;
} datatype_cast_float;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
typedef union {
  struct {
    uint64_t mantissa  :  MANTISSA_LENGTH_DOUBLE; // actually not C99 since it is a 64 Bit type
    uint32_t exponent : 11;
    uint32_t sign     : 1;
  } p;
	double f;
} datatype_cast_double;

#pragma GCC diagnostic pop


/**
 * \brief Writes dimensional information into buffer
 * \param dest Pointer to write location
 * \param dim The dimensional information
 * \pre dest != NULL
 * \return Byte size consumed of destination buffer
 */
size_t scilU_write_dims_to_buffer(void* dest, const scil_dims_t* dims);

/**
 * \brief Reads dimensional information from buffer.
 * \param dest Pointer to read location
 * \pre dest != NULL
 * \return Dimensional configuration of compressed data
 */
void scilU_read_dims_from_buffer(scil_dims_t* dims, void* dest);

////////////// TIMER MANAGEMENT /////////////////////

typedef struct timespec scil_timer;

/**
 * \brief Calculates the difference between two timestamps.
 * \param start First timestamp.
 * \param end Second timestamp.
 * \return end - start
 */
scil_timer scilU_time_diff (scil_timer end, scil_timer start);

/**
 * \brief Calculates the sum between two timestamps.
 * \param start First timestamp.
 * \param end Second timestamp.
 * \return t1 + t2
 */
scil_timer scilU_time_sum (scil_timer t1, scil_timer t2);

void scilU_start_timer(scil_timer * t1);
double scilU_stop_timer(scil_timer t1);

/**
 * \brief Prints time to file.
 *
 * Time in seconds with nanosecond precision.
 * Format: \\d+\\\.\\d{9}
 * \param time Time to print.
 * \param file
 */
void print_time (scil_timer time, FILE* file);

double scilU_time_to_double (scil_timer t);


void scilU_critical_error(const char * msg);

void scilU_print_buffer(char * dest, size_t out_size);
uint8_t scilU_relative_tolerance_to_significant_bits(double rel_tol);
double scilU_significant_bits_to_relative_tolerance(uint8_t sig_bits);

int scilU_convert_significant_decimals_to_bits(int decimals);
int scilU_convert_significant_bits_to_decimals(int bits);
int scilU_double_equal(double val1, double val2);
int scilU_float_equal(float val1, float val2);

/* Tools to iterate over the 1D buffer as a multi-dimensional data space */

typedef void(*scilG_iterfunc)(double* data,
                              const scil_dims_t* pos,
                              const scil_dims_t* size,
                              int* iter,
                              const void* user_ptr);

/*
 * \brief Convert the current position in a ND array to the position of the original 1D data array.
 */
size_t scilG_data_pos(const scil_dims_t* pos, const scil_dims_t* size);


/*
 * \brief iterate over the ND array of dimensions dims starting from offset to end in steps based on the array iter.
 * For each element the function func is invoked with the user_ptr as argument.
 */
void scilG_iter(double* data,
                const scil_dims_t* dims,
                const scil_dims_t* offset,
                const scil_dims_t* end,
                int* iter,
                scilG_iterfunc func,
                const void* user_ptr);

void scilU_print_dims(scil_dims_t dims);

void scilU_plot(const char* name, scil_dims_t dims, double * buffer_in);

// like memcopy but swaps the order
#define scilU_reverse_copy(buffer, src, size) do { char * _o = (char*) buffer; char * _s = ((char *) src) + size - 1; for(int _c=size; _c > 0; _c-- ) { *_o = *_s ; _s--; _o++; } } while(0)

#ifdef SCIL_LITTLE_ENDIAN

#define scilU_pack4(buffer, val) *((int32_t*)buffer) = val
#define scilU_unpack4(buffer, result_p) *result_p = *((int32_t*) buffer)

#define scilU_pack8(buffer, val) *((int64_t*)buffer) = val
#define scilU_unpack8(buffer, result_p) *result_p = *((int64_t*) buffer)

#else

#define scilU_pack4(buffer, val) scilU_reverse_copy(buffer, &val, 4)
#define scilU_unpack4(buffer, result_p) scilU_reverse_copy(result_p, buffer, 4)

#define scilU_pack8(buffer, val)  scilU_reverse_copy(buffer, &val, 8)
#define scilU_unpack8(buffer, result_p) scilU_reverse_copy(result_p, buffer, 8)

#endif

#endif /* SCIL_UTIL_H */
