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
/**
 * \file
 * \brief Header containing the Scientific Compression Interface Library
 * \author Julian Kunkel <juliankunkel@googlemail.com>
 * \author Armin Schaare <3schaare@informatik.uni-hamburg.de>
 */

#ifndef SCIL_HEADER_
#define SCIL_HEADER_

#include <stdio.h>
#include <stdint.h>

#include <scil-util.h>

enum SCIL_Datatype{
  SCIL_FLOAT,
  SCIL_DOUBLE
};

/*
 This amount of data may be needed for a block header.
 */
#define SCIL_BLOCK_HEADER_MAX_SIZE 1024

// These values define that the particular metrics is not of interest
#define SCIL_ACCURACY_DBL_IGNORE 0.0
#define SCIL_ACCURACY_INT_IGNORE 0

// These values define the limit for the accuracy
#define SCIL_ACCURACY_DBL_FINEST 1e-307
#define SCIL_ACCURACY_INT_FINEST -1
#define SCIL_ACCURACY_SIGNIFICANT_FINEST 16

typedef unsigned char byte;

/**
 * \brief Struct containing information on the tolerable
 * precision loss on compression
 */
typedef struct{

  /** \brief relative tolerable error (1 means 1%) */
  double relative_tolerance_percent;

  /**
   * \brief With a relative tolerance small numbers may be problematic, e.g. 1% for 0.01 becomes 0.01 +- 0.0001
   * the finest tolerance limits the smallest relative error
   * e.g. when compressing the value 0.01 with a finest absolute tolerance of 0.01 it becomes 0.01 +- 0.01
   * So this is the lower bound of the resolution and guaranteed error for relative errors,
   * where as the absolute tolerance is the guaranteed resolution for all data points.
   */
  double relative_err_finest_abs_tolerance;

  /** \brief absolute tolerable error (e.g. 1 means the value 2 can become 1-3) */
  double absolute_tolerance;

  /** \brief Number of significant digits in decimal */
  int significant_digits;

  /** \brief Alternative to the decimal digits */
  int significant_bits;

  /** \brief */
  char * force_compression_methods;

} scil_hints;


// The structure is hidden in the internal header
#ifdef SCIL_INTERNAL_HEADER_
//#warning USING INTERNAL HEADER
#else
struct scil_context_t{
    void * tmp;
};
#endif

typedef struct scil_context_t scil_context;

/*
 \brief Returns the number of available compression schemes.
 */
int scil_compressors_available();
const char * scil_compressor_name(int num);

scil_dims_t scil_init_dims(const uint8_t dimensions_count, size_t* dimensions_length);

/*
 * \brief Method to get the number of actual data points in multidimensional
 * data.
 * \param dims Information about the dimensional configuration of the data
 * \return Number of data points in the data
 */
size_t scil_get_data_count(const scil_dims_t dims);

void scil_init_hints(scil_hints * hints);

void scil_hints_print(scil_hints * hints);

/*
 *
 */
uint8_t scil_retrieve_compression_magic_number(const char * str);

/**
 * \brief Creation of a compression context
 * \param out_ctx reference to the created context
 * \param hints information on the tolerable error margin
 * \pre hints != NULL
 * \return success state of the creation
 */
int scil_create_compression_context(scil_context ** out_ctx, scil_hints * hints);

// scil_create_compression_context(scil_context ** out_ctx)
// scil_context_add_hint(ctx, char * key, char * value)

/**
 * \brief Method to compress a data buffer
 * \param datatype The datatype of the data (float, double, etc...)
 * \param dest Destination of the compressed buffer
 * \param dest_size Reference to the compressed buffer byte size, max size is given as argument, if a pipeline is processed it should be 3x the memory size of the input size + the HEADER.
 * \param source Source buffer of the data to compress
 * \param dims struct containing information about dimension count and length of buffer in each dimension
 * \param ctx Reference to the compression context
 * \pre datatype == 0 || datatype == 1
 * \pre dest != NULL
 * \pre dest_size != NULL
 * \pre source != NULL
 * \pre ctx != NULL
 * \return Success state of the compression
 */
int scil_compress(enum SCIL_Datatype datatype, byte* restrict dest, size_t dest_size,
  void*restrict source, scil_dims_t dims, size_t* restrict out_size, scil_context* ctx);

/**
 * \brief Method to decompress a data buffer
 * \param datatype The datatype of the data (float, double, etc...)
 * \param dest Destination of the decompressed buffer
 * \param expected_dims Dimensional information about the decompressed buffer
 * \param source Source buffer of data to decompress
 * \param source_size Byte size of compressed data source buffer
 * \pre datatype == 0 || datatype == 1
 * \pre dest != NULL
 * \pre source != NULL
 * \return Success state of the decompression
 */
int scil_decompress(enum SCIL_Datatype datatype, void*restrict dest, scil_dims_t expected_dims,
    byte*restrict source, const size_t source_size);

void scil_determine_accuracy(enum SCIL_Datatype datatype, const void * restrict  data_1,
    const void * restrict data_2, scil_dims_t dims,
    const double relative_err_finest_abs_tolerance, scil_hints * out_hints);

/**
 \brief Test method: check if the conditions as specified by ctx are met by comparing compressed and decompressed data.
 out_accuracy contains a set of hints with the observed finest resolution/required precision to accept the data.
 */
int scil_validate_compression(enum SCIL_Datatype datatype,
                             const void*restrict data_uncompressed,
                             scil_dims_t dims,
                             byte*restrict data_compressed,
                             const size_t compressed_size,
                             const scil_context* ctx,
                             scil_hints * out_accuracy);

#endif
