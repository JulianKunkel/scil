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

#include <stdint.h>
#include <stdlib.h>

typedef unsigned char byte;

enum SCIL_Datatype{
  SCIL_TYPE_FLOAT,
  SCIL_TYPE_DOUBLE
};

enum scil_error_code{
  SCIL_NO_ERR   = 0,
  SCIL_BUFFER_ERR,
  SCIL_MEMORY_ERR,
  SCIL_EINVAL,
  SCIL_UNKNOWN_ERR
};

enum scil_performance_unit{
  SCIL_PERFORMANCE_IGNORE = 0,
  SCIL_PERFORMANCE_MIB,
  SCIL_PERFORMANCE_GIB,
  SCIL_PERFORMANCE_NETWORK, // this unit indicates the performance of the network interconnect, e.g., Infiniband
  SCIL_PERFORMANCE_NODELOCAL_STORAGE, // the performance of the local storage
  SCIL_PERFORMANCE_SINGLESTREAM_SHARED_STORAGE  // this unit indicates the performance of one thread sending data to the shared storage, e.g., 1 GiB/s with Lustre
};

/* describes the required performance, it consists of a base unit and a multiplier, the result is "multiplier * unit" */
typedef struct{
  enum scil_performance_unit unit;
  float multiplier;
} scil_performance_hint_t;

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

  /** Describes the performance requirements for the compressors */
  scil_performance_hint_t comp_speed;
  scil_performance_hint_t decomp_speed;

  /** \brief */
  char * force_compression_methods;

} scil_hints;

typedef struct scil_context_t* scil_context_p;

/** \brief Struct to contain the dimensional configuration of data. */
typedef struct {
    /** \brief Number of dimensions. */
    uint8_t dims;

    // dimensions as long as they are <= 4
    size_t length[4];
} scil_dims;

const char* scil_strerr(enum scil_error_code error);

/*
 \brief Returns the number of available compression schemes.
 */
int scil_compressors_available();
const char * scil_compressor_name(int num);
int scil_compressor_num_by_name(const char * name);

void scil_compression_sprint_last_algorithm_chain(scil_context_p ctx, char * out, int buff_length);

void scil_init_dims_1d(scil_dims* dims, size_t dim1);
void scil_init_dims_2d(scil_dims* dims, size_t dim1, size_t dim2);
void scil_init_dims_3d(scil_dims* dims, size_t dim1, size_t dim2, size_t dim3);
void scil_init_dims_4d(scil_dims* dims, size_t dim1, size_t dim2, size_t dim3, size_t dim4);

/*
 */
void scil_init_dims_array(scil_dims* dims, uint8_t dimensions_count, const size_t* dimensions_length);

/*
 * \brief Method to get the number of actual data points in multidimensional
 * data.
 * \param dims Information about the dimensional configuration of the data
 * \return Number of data points in the data
 */
size_t scil_get_data_count(const scil_dims* dims);

/*
 * \brief Return the minimum size of the compression buffer needed.
 */
size_t scil_compress_buffer_size_bound(enum SCIL_Datatype datatype, const scil_dims* dims);

/**
 * \brief Initialize the data structure with the valid hints that are relaxed
 */
void scil_init_hints(scil_hints * hints);

scil_hints scil_retrieve_effective_hints(scil_context_p ctx);

void scil_hints_print(const scil_hints * hints);

/**
 * \brief Creation of a compression context
 * \param datatype The datatype of the data (float, double, etc...)
 * \param out_ctx reference to the created context
 * \param hints information on the tolerable error margin
 * \pre hints != NULL
 * \return success state of the creation
 */
int scil_create_compression_context(scil_context_p  * out_ctx, enum SCIL_Datatype datatype, const scil_hints * hints);

int scil_destroy_compression_context(scil_context_p  * out_ctx);

/**
 * \brief Method to compress a data buffer
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
int scil_compress(byte* restrict dest, size_t dest_size,
  void*restrict source, scil_dims* dims, size_t* restrict out_size, scil_context_p  ctx);

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
 * \pre tmp_buff != NULL with a size of scil_compress_buffer_size_bound() / 2
 * \return Success state of the decompression
 */
int scil_decompress(enum SCIL_Datatype datatype, void*restrict dest, scil_dims * const expected_dims,
    byte*restrict source, const size_t source_size, byte*restrict tmp_buff);

void scil_determine_accuracy(enum SCIL_Datatype datatype, const void * restrict  data_1,
    const void * restrict data_2, scil_dims* dims,
    const double relative_err_finest_abs_tolerance, scil_hints * out_hints);

/**
 \brief Test method: check if the conditions as specified by ctx are met by comparing compressed and decompressed data.
 out_accuracy contains a set of hints with the observed finest resolution/required precision to accept the data.
 */
int scil_validate_compression(enum SCIL_Datatype datatype,
                             const void*restrict data_uncompressed,
                             scil_dims* dims,
                             byte*restrict data_compressed,
                             const size_t compressed_size,
                             const scil_context_p  ctx,
                             scil_hints * out_accuracy);

#endif
