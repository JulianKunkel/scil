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
 * \brief Header containing zfp with absolute error tolerance for the Scientific
 * Compression Interface Library
 * \author Armin Schaare <3schaare@informatik.uni-hamburg.de>
 */

#ifndef SCIL_ZFP_ABSTOL_H_
#define SCIL_ZFP_ABSTOL_H_

#include <scil-algorithm-impl.h>

//Supported datatypes: float double
// Repeat for each data type
/**
 * \brief Compression function of zfp with absolute error tolerance
 * \param ctx Compression context used for this compression
 * \param dest Preallocated buffer which will hold the compressed data
 * \param dest_size Byte size the compressed buffer will have
 * \param source Uncompressed data which should be processed
 * \param source_count Element count of uncompressed buffer
 * \return Success state of the compression
 */
int scil_zfp_abstol_compress_<DATATYPE>(const scil_context_t* ctx, byte* restrict dest, size_t* restrict dest_size, <DATATYPE>*restrict source, const scil_dims_t* dims);

/**
 * \brief Decompression function of zfp with absolute error tolerance
 * \param data_out Pre allocated buffer which will hold the decompressed data
 * \param dims Dimensional layout of decompressed buffer
 * \param compressed_buf_in Buffer holding data to be decompressed
 * \param in_size Byte size of compressed_buf_in
 * \return Success state of the compression
 */
int scil_zfp_abstol_decompress_<DATATYPE>( <DATATYPE>*restrict data_out, scil_dims_t* dims, byte*restrict compressed_buf_in, const size_t in_size);

// End repeat


extern scilU_algorithm_t algo_zfp_abstol;

#endif /* SCIL_ZFP_ABSTOL_H_<DATATYPE> */
