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
 * \author Armin Schaare <3schaare@informatik.uni-hamburg.de>
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <scil-dims.h>
#include <scil-util-types.h>

/**
 * \brief Makro to make sure off_t is 64-bit unsigned integer
 */
#define _FILE_OFFSET_BITS 64

#define MANTISSA_MAX_LENGTH 52
#define MANTISSA_MAX_LENGTH_P1 65
#define MANTISSA_LENGTH_FLOAT 23
#define MANTISSA_LENGTH_DOUBLE 52

#define EXPONENT_LENGTH_FLOAT (32 - MANTISSA_LENGTH_FLOAT)
#define EXPONENT_LENGTH_DOUBLE (64 - MANTISSA_LENGTH_DOUBLE)

#define max(a, b) \
  (a > b ? a : b)

#define min(a, b) \
  (-max(-a, -b))

#define DATATYPE_LENGTH(type) (type == SCIL_TYPE_FLOAT ? sizeof(float) : type == SCIL_TYPE_DOUBLE ? sizeof(double) : type == SCIL_TYPE_INT8 ? sizeof(int8_t) : type == SCIL_TYPE_INT16 ? sizeof(int16_t) : type == SCIL_TYPE_INT32 ? sizeof(int32_t) : type == SCIL_TYPE_INT64 ? sizeof(int64_t) : 1)

void *scilU_safe_malloc(size_t size);

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

/*
 * \brief Returns the byte size of data with its dimensional configuration given by dims.
 * data.
 * \param dims Dimensional configuration of the data
 * \param type The datas type (i.e. float, double, etc.)
 * \return Byte size of the data
 */
size_t scil_dims_get_size(const scil_dims_t *dims, enum SCIL_Datatype type);

/*
 * \brief Return the minimum size of the compression buffer needed.
 */
size_t scil_get_compressed_data_size_limit(const scil_dims_t *dims, enum SCIL_Datatype datatype);

/**
 * \brief Writes dimensional information into buffer
 * \param dest Pointer to write location
 * \param dim The dimensional information
 * \pre dest != NULL
 * \return Byte size consumed of destination buffer
 */
size_t scilU_write_dims_to_buffer(void *dest, const scil_dims_t *dims);

/**
 * \brief Reads dimensional information from buffer.
 * \param dest Pointer to read location
 * \pre dest != NULL
 * \return Dimensional configuration of compressed data
 */
void scilU_read_dims_from_buffer(scil_dims_t *dims, void *dest);

////////////// TIMER MANAGEMENT /////////////////////

typedef struct timespec scil_timer;

/**
 * \brief Calculates the difference between two timestamps.
 * \param start First timestamp.
 * \param end Second timestamp.
 * \return end - start
 */
scil_timer scilU_time_diff(scil_timer end, scil_timer start);

/**
 * \brief Calculates the sum between two timestamps.
 * \param start First timestamp.
 * \param end Second timestamp.
 * \return t1 + t2
 */
scil_timer scilU_time_sum(scil_timer t1, scil_timer t2);

void scilU_start_timer(scil_timer *t1);
double scilU_stop_timer(scil_timer t1);

/**
 * \brief Prints time to file.
 *
 * Time in seconds with nanosecond precision.
 * Format: \\d+\\\.\\d{9}
 * \param time Time to print.
 * \param file
 */
void print_time(scil_timer time, FILE *file);

double scilU_time_to_double(scil_timer t);

void scilU_print_buffer(char *dest, size_t out_size);
uint8_t scilU_relative_tolerance_to_significant_bits(double rel_tol);
double scilU_significant_bits_to_relative_tolerance(uint8_t sig_bits);

int scilU_convert_significant_decimals_to_bits(int decimals);
int scilU_convert_significant_bits_to_decimals(int bits);
int scilU_double_equal(double val1, double val2);
int scilU_float_equal(float val1, float val2);

void scilU_find_minimum_maximum(SCIL_Datatype_t datatype,
                                byte *data,
                                scil_dims_t *dims,
                                double *out_min,
                                double *out_max);

void scilU_find_minimum_maximum_with_excluded_points(SCIL_Datatype_t datatype,
                                                     byte *data,
                                                     scil_dims_t *dims,
                                                     double *out_min,
                                                     double *out_max,
                                                     double ignore_up_to,
                                                     double ignore_from,
                                                     double fill_value);

// this function substracts data2 from data1 and stores the result in data2
void scilU_subtract_data(SCIL_Datatype_t datatype,
                         byte *restrict data1,
                         byte *restrict in_out_data2,
                         scil_dims_t *dims);

/* Tools to iterate over the 1D buffer as a multi-dimensional data space */

typedef void(*scilU_iterfunc)(double *data,
                              const scil_dims_t *pos,
                              const scil_dims_t *size,
                              int *iter,
                              const void *user_ptr);

/*
 * \brief Convert the current position in a ND array to the position of the original 1D data array.
 */
size_t scilU_data_pos(const scil_dims_t *pos, const scil_dims_t *size);

/*
 * \brief iterate over the ND array of dimensions dims starting from offset to end in steps based on the array iter.
 * For each element the function func is invoked with the user_ptr as argument.
 */
void scilU_iter(double *data,
                const scil_dims_t *dims,
                const scil_dims_t *offset,
                const scil_dims_t *end,
                int *iter,
                scilU_iterfunc func,
                const void *user_ptr);

void scilU_print_dims(scil_dims_t dims);

// like memcopy but swaps the order
#define scilU_reverse_copy(buffer, src, size) do { char * _o = (char*) buffer; char * _s = ((char *) src) + size - 1; for(int _c=size; _c > 0; _c-- ) { *_o = *_s ; _s--; _o++; } } while(0)

#define scilU_pack1(buffer, val) *((int8_t*)buffer) = val
#define scilU_unpack1(buffer, result_p) *result_p = *((int8_t*) buffer)

#ifdef SCIL_LITTLE_ENDIAN

#define scilU_pack2(buffer, val) *((int16_t*)buffer) = *(int16_t*) & val
#define scilU_unpack2(buffer, result_p) *(int16_t*)result_p = *((int16_t*) buffer)

#define scilU_pack4(buffer, val) *((int32_t*)buffer) = *(int32_t*) & val
#define scilU_unpack4(buffer, result_p) *(int32_t*)result_p = *((int32_t*) buffer)

#define scilU_pack8(buffer, val) *((int64_t*)buffer) = *(int64_t*) & val
#define scilU_unpack8(buffer, result_p) *(int64_t*) result_p = *((int64_t*) buffer)

#else

#define scilU_pack4(buffer, val) scilU_reverse_copy(buffer, &val, 4)
#define scilU_unpack4(buffer, result_p) scilU_reverse_copy(result_p, buffer, 4)

#define scilU_pack8(buffer, val)  scilU_reverse_copy(buffer, &val, 8)
#define scilU_unpack8(buffer, result_p) scilU_reverse_copy(result_p, buffer, 8)

#endif

typedef struct {
  double fill_value;
  int layout;
} special_values;

#endif /* SCIL_UTIL_H */
