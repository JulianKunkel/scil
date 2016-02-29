/**
 * \file
 * \brief Header containing gzip of the Scientific Compression Interface Library
 * \author Julian Kunkel <juliankunkel@googlemail.com>
 * \author Armin Schaare <3schaare@informatik.uni-hamburg.de>
 */

#ifndef SCIL_GZIP_H_
#define SCIL_GZIP_H_

#include <scil-internal.h>

/**
 * \brief Compression function of gzip
 * \param ctx Compression context used for this compression
 * \param dest Pre allocated buffer which will hold the compressed data
 * \param dest_size Byte size the compressed buffer will have
 * \param source Uncompressed data which should be processed
 * \param source_count Element count of uncompressed buffer
 * \return Success state of the compression
 */
int scil_gzip_compress(const scil_context* ctx, byte* restrict dest, size_t* restrict dest_size, const double*restrict source, const size_t source_count);

/**
 * \brief Deompression function of gzip
 * \param ctx Compression context used for this decompression
 * \param dest Pre allocated buffer which will hold the decompressed data
 * \param dest_count Element count of decompressed buffer
 * \param source Compressed data which should be processed
 * \param source_size Byte size of compressed buffer
 * \return Success state of the compression
 */
int scil_gzip_decompress(const scil_context* ctx, double*restrict dest, size_t*restrict dest_count, const byte*restrict source, const size_t source_size);

extern scil_compression_algorithm algo_gzip;

#endif
