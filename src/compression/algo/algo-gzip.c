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

#include <zlib.h>

#include <algo-gzip.h>

#pragma GCC diagnostic ignored "-Wunused-parameter"
int scil_gzip_compress(const scil_context_t* ctx, byte* restrict dest, size_t* restrict dest_size, const byte*restrict source, const size_t source_size){
  *dest_size = 2*source_size;
  int ret = compress( (Bytef*)dest, dest_size, (Bytef*)source, (uLong)(source_size) );
  if (ret == Z_OK){
    return SCIL_NO_ERR;
  }else{
    debug("Error in gzip compression. (Buf error: %d mem error: %d data_error: %d size: %lld)\n",
    ret == Z_BUF_ERROR , ret == Z_MEM_ERROR, ret == Z_DATA_ERROR, (long long) source_size);

    return SCIL_UNKNOWN_ERR;
  }
}

#pragma GCC diagnostic ignored "-Wunused-parameter"
int scil_gzip_decompress(byte*restrict data_out, size_t buff_size,  const byte*restrict compressed_buf_in, const size_t in_size, size_t * uncomp_size_out)
{
  uLongf out_buffer_size = (uLongf) buff_size;
  int ret = uncompress( (Bytef*)data_out, & out_buffer_size, (Bytef*)compressed_buf_in, (uLong)in_size);

  if(ret != Z_OK){
      debug("Error in gzip decompression. (Buf error: %d mem error: %d data_error: %d size: %lld)\n",
      ret == Z_BUF_ERROR , ret == Z_MEM_ERROR, ret == Z_DATA_ERROR, (long long) *uncomp_size_out);
      ret = SCIL_UNKNOWN_ERR;
  }
  *uncomp_size_out = (size_t) out_buffer_size;

  return ret;
}

scilU_algorithm_t algo_gzip = {
    .c.Btype = {
        scil_gzip_compress,
        scil_gzip_decompress
    },
    "gzip",
    2,
    SCIL_COMPRESSOR_TYPE_INDIVIDUAL_BYTES
};
