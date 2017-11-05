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
 * \brief Header containing abstol of the Scientific Compression Interface
 * Library
 * \author Julian Kunkel <juliankunkel@googlemail.com>
 * \author Armin Schaare <3schaare@informatik.uni-hamburg.de>
 */

#ifndef SCIL_ABSTOL_H_
#define SCIL_ABSTOL_H_

#include <scil-algorithm-impl.h>

//Repeat for each data type
//Supported datatypes:double float

/**
 * \brief Compression function of abstol
 * \param ctx Compression context used for this compression
 * \param dest Preallocated buffer which will hold the compressed data
 * \param dest_size Byte size the compressed buffer will have
 * \param source Uncompressed data which should be processed
 * \param source_count Element count of uncompressed buffer
 * \return Success state of the compression
 */
int scil_abstol_compress_<DATATYPE>(const scil_context_t* ctx,
                                    byte* restrict dest,
                                    size_t* restrict dest_size,
                                    <DATATYPE>* restrict source,
                                    const scil_dims_t* dims);

/**
 * \brief Decompression function of abstol
 * \param dest Pre allocated buffer which will hold the decompressed data
 * \param out_size Element count of decompressed buffer (not byte size!)
 * \param source Compressed data which should be processed
 * \param in_size Byte size of compressed buffer
 * \return Success state of the compression
 */
int scil_abstol_decompress_<DATATYPE>(<DATATYPE>* restrict dest,
                                      scil_dims_t* dims,
                                      byte* restrict source,
                                      size_t in_size);
// End repeat

extern scilU_algorithm_t algo_abstol;

#endif /* SCIL_ABSTOL_H_<DATATYPE> */
