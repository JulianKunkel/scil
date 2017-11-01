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
 * \brief Header containing gzip of the Scientific Compression Interface Library
 * \author Julian Kunkel <juliankunkel@googlemail.com>
 * \author Armin Schaare <3schaare@informatik.uni-hamburg.de>
 */

#ifndef SCIL_GZIP_H_
#define SCIL_GZIP_H_

#include <scil-algorithm-impl.h>

/**
 * \brief Compression function of gzip
 * \param ctx Compression context used for this compression
 * \param dest Pre allocated buffer which will hold the compressed data
 * \param dest_size Byte size the compressed buffer will have
 * \param source Uncompressed data which should be processed
 * \param source_size Byte size of uncompressed buffer
 * \return Success state of the compression
 */
int scil_gzip_compress(const scil_context_t* ctx, byte* restrict dest, size_t* restrict dest_size, const byte* restrict source, const size_t source_size);

/**
 * \brief Deompression function of gzip
 * \param data_out Buffer to hold the decompressed data
 * \param compressed_buf_in Buffer holding the compressed data
 * \param in_size Byte-size of the compressed buffer
 * \param uncompressed_size_out Byte-size of the decompressed buffer
 * \return Success state of the compression
 */
int scil_gzip_decompress(byte*restrict data_out, size_t buff_size, const byte*restrict compressed_buf_in, const size_t in_size, size_t * uncomp_size_out);

extern scilU_algorithm_t algo_gzip;

#endif
