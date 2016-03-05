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
#include <algo/gzip.h>

#include <zlib.h>

int scil_gzip_compress(const scil_context* ctx, byte* restrict dest, size_t* restrict dest_size, const DataType*restrict source, const size_t source_count){
  return compress( (Bytef*)dest, dest_size, (Bytef*)source, (uLong)(source_count) ) == Z_OK ;
}

int scil_gzip_decompress(const scil_context* ctx, DataType*restrict dest, size_t*restrict dest_count, const byte*restrict source, const size_t source_size){
    uLongf dest_size = *dest_count;
    int ret = uncompress( (Bytef*)dest, & dest_size, (Bytef*)source, (uLong)source_size);

    if(ret != Z_OK){
        fprintf(stderr, "Error in gzip decompression. (Buf error: %d mem error: %d data_error: %d size: %lld)\n",
        ret == Z_BUF_ERROR , ret == Z_MEM_ERROR, ret == Z_DATA_ERROR, (long long) dest_size);
    }

    *dest_count = dest_size;
    return ret;
}

scil_compression_algorithm algo_gzip = {
    scil_gzip_compress,
    scil_gzip_decompress,
    "gzip",
    2,
    SCIL_COMPRESSOR_TYPE_INDIVIDUAL_BYTES
};
