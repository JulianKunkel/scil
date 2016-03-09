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

int scil_memcopy_compress(const scil_context* ctx, byte* restrict dest, size_t*restrict dest_size, const byte*restrict source, const size_t source_count){
    // TODO check if out_size is sufficently large
    *dest_size = source_count;
    memcpy(dest, source, source_count);
    return 0;
}

int scil_memcopy_decompress(const scil_context* ctx, byte*restrict dest, size_t*restrict dest_count, const byte*restrict source, const size_t source_size){
    // TODO check if buff is sufficiently large
    *dest_count = source_size;
    memcpy(dest, source, source_size);
    return 0;
}

scil_compression_algorithm algo_memcopy = {
    .c.btype = {
      scil_memcopy_compress,
      scil_memcopy_decompress
    },
    "memcopy",
    0,
    SCIL_COMPRESSOR_TYPE_INDIVIDUAL_BYTES
};
