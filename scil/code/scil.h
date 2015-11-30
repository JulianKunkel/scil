/**
 * \file 
 * \brief Header containing the Scientific Compression Interface Library
 * \author Julian Kunkel <juliankunkel@googlemail.com>
 * \author Armin Schaare <3schaare@informatik.uni-hamburg.de>
 */

#ifndef SCIL_HEADER_
#define SCIL_HEADER_

#include <stdio.h>

/**
 * \brief Struct containing information on the tolerable
 * precision loss on compression 
 */
typedef struct{
  /** \brief relative tolerable error (1 means 1%) */
  double relative_tolerance_percent;

  /** with a relative tolerance small numbers may be problematic, e.g. 1% for 0.01 becomes 0.01 +- 0.0001 
      the finest tolerance limits the smallest relative error 
      e.g. when compressing the value 0.01 with a filest abs tolerance of 0.01 it becomes 0.01 +- 0.01
      So this is the lower bound of the resolution and guaranteed error for relative errors, 
      where as the absolute tolerance is the guaranteed resolution for all data points.
  **/
  double relative_err_finest_abs_tolerance;

  /** \brief absolute tolerable error (e.g. 1 means the value 2 can become 1-3) */
  double absolute_tolerance;

  /** \brief Number of significant digits */
  int significant_digits;
  
  int force_compression_method;
  
  uint64_t force_block_size;
} scil_hints;


// The structure is hidden in the internal header
#ifdef SCIL_INTERNAL_HEADER_
#warning USING INTERNAL HEADER
#else
struct scil_context_t{ 
    void * tmp;
};
#endif

typedef struct scil_context_t scil_context;

/**
 * \brief Creation of a compression context
 * \param out_ctx reference to the created context
 * \param hints information on the tolerable error margin
 * \pre hints != NULL
 * \return success state of the creation
 */
int scil_create_compression_context(scil_context ** out_ctx, scil_hints * hints);

/**
 * \brief Compression method of a buffer of data
 * \param ctx reference of the compression context
 * \param compressed_buf_out reference to the compressed buffer
 * \param out_size reference to the compressed buffer byte size, max size is given as argument
 * \param data_in buffer of data to compress
 * \param in_size count of numbers in data buffer
 * \pre ctx != NULL
 * \pre data_in != NULL
 * \return success state of the compression
 */
int scil_compress(scil_context* ctx, char** restrict compressed_buf_out, size_t* restrict out_size, const double*restrict data_in, const size_t in_size);

/**
 * \brief Decompression method of a buffer of data
 * \param ctx reference of the compression context
 * \param data_out reference to the decompressed buffer
 * \param out_size reference to the decompressed buffer element count
 * \param compressed_buf_in buffer of data to decompress
 * \param in_size byte size of compressed data buffer
 * \pre ctx != NULL
 * \pre compressed_buf_in != NULL
 * \return success state of the decompression
 */
int scil_decompress(const scil_context* ctx, double*restrict data_out, size_t*restrict out_size, const char*restrict compressed_buf_in, const size_t in_size);


/**
 \brief Test method: check if the conditions as specified by ctx are met by comparing compressed and decompressed data.
 */
int scil_validate_compression(const scil_context* ctx,
                             const size_t uncompressed_size,
                             const double*restrict data_uncompressed,
                             const size_t compressed_size,
                             const double*restrict data_compressed );

#endif
