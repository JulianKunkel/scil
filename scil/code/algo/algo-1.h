/**
 * \file
 * \brief Header containing algo1 of the Scientific Compression Interface Library
 * \author Julian Kunkel <juliankunkel@googlemail.com>
 * \author Armin Schaare <3schaare@informatik.uni-hamburg.de>
 */

#ifndef SCIL_ALGO1_H_
#define SCIL_ALGO1_H_

#include <scil-internal.h>

/**
 * \brief Compression function of algo1
 * \param ctx Compression context used for this compression
 * \param compressed_buf_out Pre allocated buffer which will hold the compressed data
 * \param out_size Byte size the compressed buffer will have
 * \param data_in Uncompressed data which should be processed
 * \param in_size Element count of uncompressed buffer (not byte size!)
 * \return Success state of the compression
 */
int scil_algo1_compress(const scil_context* ctx, char** restrict compressed_buf_out, size_t* restrict out_size, const double*restrict data_in, const size_t in_size);

/**
 * \brief Deompression function of algo1
 * \param ctx Compression context used for this decompression
 * \param data_out Pre allocated buffer which will hold the decompressed data
 * \param out_size Element count of decompressed buffer (not byte size!)
 * \param compressed_buf_in Compressed data which should be processed
 * \param in_size Byte size of compressed buffer
 * \return Success state of the compression
 */
int scil_algo1_decompress(const scil_context* ctx, double*restrict data_out, size_t*restrict out_size, const char*restrict compressed_buf_in, const size_t in_size);

scil_compression_algorithm algo_algo1 = {
    scil_algo1_compress,
    scil_algo1_decompress,
    "algo1",
    1
};

#endif
