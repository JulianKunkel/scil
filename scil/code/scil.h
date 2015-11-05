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
 * \brief Struct containing information on the tolerable precision
 * loss on compression 
 */
typedef struct{
  /** \brief relative tolerable error (1 means 1%) */
  double relative_tolerance_percent;
  /** \brief absolute tolerable error (e.g. 1 means the value 2 can become 1-3) */
  double absolute_tolerance;
} scil_hints;


// TODO later: hide the structure in an internal header
typedef struct{
  scil_hints hints;
  //...
} scil_context;

/**
 * \brief Creation of a compression context
 * \param out_ctx reference to the created context
 * \param hints information on the tolerable error margin
 * \pre hints != NULL
 * \return success state of the creation
 */
int scil_create_compression_context(scil_context * out_ctx, scil_hints * hints);

/**
 * \brief Compression method of a buffer of data
 * \param ctx reference of the compression context
 * \param compressed_buf_out reference to the compressed buffer
 * \param out_size reference to the compressed buffer byte size
 * \param data_in buffer of data to compress
 * \param in_size count of numbers in data buffer
 * \pre ctx != NULL
 * \pre data_in != NULL
 * \return success state of the compression
 */
int scil_compress(const scil_context* ctx, char** compressed_buf_out, size_t* out_size, const double* data_in, const size_t in_size);

/**
 * \brief Deompression method of a buffer of data
 * \param ctx reference of the compression context
 * \param data_out reference to the decompressed buffer
 * \param out_size reference to the decompressed buffer element count
 * \param compressed_buf_in buffer of data to decompress
 * \param in_size byte size of compressed data buffer
 * \pre ctx != NULL
 * \pre compressed_buf_in != NULL
 * \return success state of the decompression
 */
int scil_decompress(const scil_context* ctx, double* data_out, const size_t* out_size, const char* compressed_buf_in, const size_t in_size);

#endif
