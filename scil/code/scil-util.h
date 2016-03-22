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
#ifndef UTIL_H
#define UTIL_H

/**
 * \file
 * \brief Contains miscellanious useful functions.
 * \author Lennart Braun <3braun@informatik.uni-hamburg.de>
 * \author Armin Schaare <3schaare@informatik.uni-hamburg.de>
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/**
 * \brief Makro to make sure off_t is 64-bit unsigned integer
 */
#define _FILE_OFFSET_BITS 64

#define max(a,b) \
  (a > b ? a : b)

#define min(a,b) \
  (-max(-a, -b))

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
void* safe_calloc (size_t nmemb, size_t size, const char* file,
                   unsigned long line);


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
void* safe_realloc (void* ptr, size_t size, const char* file,
                    unsigned long line);


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


/** \brief Struct to contain the dimensional configuration of data. */
typedef struct {
    /** \brief Number of dimensions. */
    uint8_t dims;

    /**
     * \brief Lengths of each dimension.
     * The caller is responsible to free it.
     */
    size_t * length;
} SCIL_dims_t;


/**
 * \brief Writes dimensional information into buffer
 * \param dest Pointer to write location
 * \param dim The dimensional information
 * \pre dest != NULL
 * \return Byte size consumed of destination buffer
 */
size_t scil_write_dims_to_buffer(void* dest, const SCIL_dims_t dims);

/**
 * \brief Reads dimensional information from buffer. Allocates memory for
 * dims.length!
 * \param dest Pointer to read location
 * \pre dest != NULL
 * \return Dimensional configuration of compressed data
 */
SCIL_dims_t scil_read_dims_from_buffer(void* dest);

/**
 * \brief Calculates the difference between two timestamps.
 * \param start First timestamp.
 * \param end Second timestamp.
 * \return end - start
 */
struct timespec diff_timespec (struct timespec start, struct timespec end);

/**
 * \brief Calculates the sum between two timestamps.
 * \param start First timestamp.
 * \param end Second timestamp.
 * \return t1 + t2
 */
struct timespec sum_timespec (struct timespec t1, struct timespec t2);

/**
 * \brief Calculates the quotient of a time and an integer
 * \param t First timestamp.
 * \param d divisor
 * \return t / d
 */
struct timespec div_timespec (struct timespec t, int64_t d);

/**
 * \brief Prints time to file.
 *
 * Time in seconds with nanosecond precision.
 * Format: \\d+\\\.\\d{9}
 * \param time Time to print.
 * \param file
 */
void print_time (struct timespec time, FILE* file);

double timespec_to_double (struct timespec t);

uint64_t timespec_to_uint64 (struct timespec t);

void critical_error(const char * msg);


#endif /* UTIL_H */
