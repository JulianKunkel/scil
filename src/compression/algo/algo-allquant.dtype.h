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
 * \brief Header containing allquant of the Scientific Compression Interface
 * Library
 * \author Julian Kunkel <juliankunkel@googlemail.com>
 * \author Oliver Pola <5pola@informatik.uni-hamburg.de>
 */

#ifndef SCIL_ALL_QUANTITIES_H_
#define SCIL_ALL_QUANTITIES_H_

#include <scil-algorithm-impl.h>

//Repeat for each data type
//Supported datatypes: double float int8_t int16_t int32_t int64_t

int scil_allquant_compress_<DATATYPE>(const scil_context_t* ctx,
                                    byte* restrict dest,
                                    size_t* restrict dest_size,
                                    <DATATYPE>* restrict source,
                                    const scil_dims_t* dims);

int scil_allquant_decompress_<DATATYPE>(<DATATYPE>* restrict dest,
                                      scil_dims_t* dims,
                                      byte* restrict source,
                                      size_t in_size);
// End repeat

extern scilU_algorithm_t algo_allquant;

#endif
