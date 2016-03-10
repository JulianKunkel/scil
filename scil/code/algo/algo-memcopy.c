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

#include <algo/algo-memcopy.h>

#include <string.h>

#pragma GCC diagnostic ignored "-Wunused-parameter"
int scil_memcopy_compress(const scil_context* ctx, byte* restrict dest, uint64_t * restrict out_size, const byte*restrict source, const size_t source_size){
    // TODO check if out_size is sufficently large
    *out_size = source_size;
    memcpy(dest, source, source_size);
    return 0;
}

#pragma GCC diagnostic ignored "-Wunused-parameter"
int scil_memcopy_decompress(const scil_context* ctx, byte*restrict dest, uint64_t exp_size, const byte*restrict source, const size_t source_size){
    // TODO check if buff is sufficiently large
    memcpy(dest, source, source_size);
    return 0;
}

scil_compression_algorithm algo_memcopy = {
    .c.Btype = {
        scil_memcopy_compress,
        scil_memcopy_decompress
    },
    "memcopy",
    0,
    SCIL_COMPRESSOR_TYPE_INDIVIDUAL_BYTES
};
