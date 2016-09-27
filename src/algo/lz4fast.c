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

#include <algo/lz4fast.h>

#include <string.h>
#include <lz4.h>

#pragma GCC diagnostic ignored "-Wunused-parameter"
int scil_lz4fast_compress(const scil_context_t* ctx, byte* restrict dest, size_t * restrict out_size, const byte*restrict source, const size_t source_size){
    int size;
    // store the size of the data
    *((int*) dest) = source_size;
    size = LZ4_compress ((const char *) (source), (char *) dest + 4, source_size);
    *out_size = size + 4;

    if (size == 0){
      return -1;
    }

    return 0;
}

#pragma GCC diagnostic ignored "-Wunused-parameter"
int scil_lz4fast_decompress(byte*restrict dest, size_t buff_size, const byte*restrict src, const size_t in_size, size_t * uncomp_size_out){
    int size;
    // retrieve the size of the uncompressed data
    size = *((int*) src);
    *uncomp_size_out = size;
    size = LZ4_decompress_fast((const char *) src + 4, (char *)  dest, size); // LZ4_decompress_safe (source, dest, source_size, exp_size);

    return 0;
}

scilI_algorithm_t algo_lz4fast = {
    .c.Btype = {
        scil_lz4fast_compress,
        scil_lz4fast_decompress
    },
    "lz4",
    7,
    SCIL_COMPRESSOR_TYPE_INDIVIDUAL_BYTES
};
