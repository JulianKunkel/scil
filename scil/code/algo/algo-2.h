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
 * \brief Header containing algo1 of the Scientific Compression Interface Library
 * \author Armin Schaare <3schaare@informatik.uni-hamburg.de>
 */

#ifndef SCIL_ALGO2_H_
#define SCIL_ALGO2_H_

#include <scil-internal.h>

/**
 * \brief Compression function of algo2
 * \param ctx Compression context used for this compression
 * \param dest Preallocated buffer which will hold the compressed data
 * \param dest_size Byte size the compressed buffer will have
 * \param source Uncompressed data which should be processed
 * \param source_count Element count of uncompressed buffer
 * \return Success state of the compression
 */
int scil_algo2_compress(const scil_context* ctx, byte* restrict dest, size_t* restrict dest_size, const DataType*restrict source, const size_t source_count);

/**
 * \brief Deompression function of algo2
 * \param ctx Compression context used for this decompression
 * \param dest Pre allocated buffer which will hold the decompressed data
 * \param out_size Element count of decompressed buffer (not byte size!)
 * \param compressed_buf_in Compressed data which should be processed
 * \param in_size Byte size of compressed buffer
 * \return Success state of the compression
 */
int scil_algo2_decompress(const scil_context* ctx, DataType*restrict dest, size_t*restrict out_size, const byte*restrict compressed_buf_in, const size_t in_size);

extern scil_compression_algorithm algo_algo2;

#endif /* SCIL_ALGO2_H_ */
