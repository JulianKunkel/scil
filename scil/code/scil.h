/**
 * \file
 * \brief Header containing the Scientific Compression Interface Library
 * \author Julian Kunkel <juliankunkel@googlemail.com>
 * \author Armin Schaare <3schaare@informatik.uni-hamburg.de>
 */

#ifndef SCIL_HEADER_
#define SCIL_HEADER_

#include <stdio.h>

/*
 This amount of data may be needed for a block header.
 */
#define SCIL_BLOCK_HEADER_MAX_SIZE 128

typedef unsigned char byte;
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

  /** \brief */
  int force_compression_method;

  //uint64_t force_block_size;
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


/*
 *
 */
int scil_retrieve_compression_magic_number(const char * str);

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
 * \brief Compression method of a buffer of data
 * \param ctx Reference to the compression context
 * \param dest Destination of the compressed buffer
 * \param dest_size Reference to the compressed buffer byte size, max size is given as argument
 * \param source Source buffer of the data to compress
 * \param source_count Count of elements in source buffer
 * \pre ctx != NULL
 * \pre dest != NULL
 * \pre dest_size != NULL
 * \pre source != NULL
 * \return Success state of the compression
 */
int scil_compress(scil_context* ctx, byte* restrict dest, size_t* restrict dest_size, const double*restrict source, const size_t source_count);

/**
 * \brief Decompression method of a buffer of data
 * \param ctx Reference to the compression context
 * \param dest Destination of the decompressed buffer
 * \param dest_count Reference to the decompressed buffer element count
 * \param source Source buffer of data to decompress
 * \param source_size Byte size of compressed data source buffer
 * \pre dest != NULL
 * \pre dest_count != NULL
 * \pre source != NULL
 * \return Success state of the decompression
 */
int scil_decompress(double*restrict dest, size_t*restrict dest_count, const byte*restrict source, const size_t source_size);

// int scil_decompress_start_stream(scil_context ** out_ctx, const char*restrict compressed_buf_in);
// int scil_decompress_stream(const scil_context* ctx, double*restrict data_out, size_t*restrict out_size, const char*restrict compressed_buf_in, const size_t in_size);


/**
 \brief Test method: check if the conditions as specified by ctx are met by comparing compressed and decompressed data.
 */
int scil_validate_compression(const scil_context* ctx,
                             const size_t uncompressed_size,
                             const double*restrict data_uncompressed,
                             const size_t compressed_size,
                             const double*restrict data_compressed );

#endif
