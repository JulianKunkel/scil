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

#include <algo/algo-gzip.h>

#include <zlib.h>

#pragma GCC diagnostic ignored "-Wunused-parameter"
int scil_gzip_compress(const scil_context* ctx, byte* restrict dest, size_t* restrict dest_size, const byte*restrict source, const size_t source_size){

  return compress( (Bytef*)dest, dest_size, (Bytef*)source, (uLong)(source_size) ) != Z_OK ;
}

#pragma GCC diagnostic ignored "-Wunused-parameter"
int scil_gzip_decompress(byte*restrict data_out, const byte*restrict compressed_buf_in, const size_t in_size, size_t * uncomp_size_out){

    uLongf dest_isize = *uncomp_size_out;
    int ret = uncompress( (Bytef*)data_out, & dest_isize, (Bytef*)compressed_buf_in, (uLong)in_size);

    if(ret != Z_OK){
        fprintf(stderr, "Error in gzip decompression. (Buf error: %d mem error: %d data_error: %d size: %lld)\n",
        ret == Z_BUF_ERROR , ret == Z_MEM_ERROR, ret == Z_DATA_ERROR, (long long) *uncomp_size_out);
        ret = -1;
    }

    return ret;
}

scil_compression_algorithm algo_gzip = {
    .c.Btype = {
        scil_gzip_compress,
        scil_gzip_decompress
    },
    "gzip",
    2,
    SCIL_COMPRESSOR_TYPE_INDIVIDUAL_BYTES
};
