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

#include <algo/algo-fpzip.h>

#include <fpzip.h>

// compress 'inbytes' bytes from 'data' to stream 'FPZ'
static int
compress(FPZ* fpz, void* data)
{
    // fpzip_write_header returns 1 on success and 0 on failure
  if (!fpzip_write_header(fpz)) {
    fprintf(stderr, "Cannot write header in algo-fpzip compression: %s\n", fpzip_errstr[fpzip_errno]);
    return 1;
  }

  // perform actual compression
  size_t outbytes = fpzip_write(fpz, data);
  if (!outbytes) {
    fprintf(stderr, "Compression failed in algo-fpzip compression: %s\n", fpzip_errstr[fpzip_errno]);
    return 1;
  }

  return 0;
}

// decompress 'inbytes' bytes to 'data' from stream 'FPZ'
static int
decompress(FPZ* fpz, void* data, size_t inbytes)
{
    // fpzip_read_header returns 1 on success and 0 on failure
  if (!fpzip_read_header(fpz)) {
    fprintf(stderr, "Cannot read header in algo-fpzip decompression: %s\n", fpzip_errstr[fpzip_errno]);
    return 1;
  }

  // make sure array size stored in header matches expectations
  size_t size = (fpz->type == FPZIP_TYPE_FLOAT ? sizeof(float) : sizeof(double));
  if (size * fpz->nx * fpz->ny * fpz->nz * fpz->nf != inbytes) {
    fprintf(stderr, "Array size does not match dimensions from header in algo-fpzip decompression.\n");
    return 1;
  }

  // perform actual decompression
  if (!fpzip_read(fpz, data)) {
    fprintf(stderr, "Algo-fpzip decompression failed: %s\n", fpzip_errstr[fpzip_errno]);
    return 1;
  }

  return 0;
}

#pragma GCC diagnostic ignored "-Wunused-parameter"
int scil_fpzip_compress(const scil_context* ctx,
                        byte * restrict dest,
                        size_t* restrict dest_size,
                        const DataType*restrict source,
                        const size_t source_count)
{
    FPZ* fpz = fpzip_write_to_buffer(dest, *dest_size);
    fpz->type = -(SCIL_DATA_TYPE-1); // in fpzip float is 0 and double 1
    fpz->prec = 0; // lossless
    fpz->nx = source_count; // one-dimensional
    fpz->ny = 1; //
    fpz->nz = 1; //
    fpz->nf = 1; //

    int ret = compress(fpz, (void*)source);

    fpzip_write_close(fpz);

    return ret;
}

#pragma GCC diagnostic ignored "-Wunused-parameter"
int scil_fpzip_decompress(  const scil_context* ctx,
                            DataType*restrict dest,
                            size_t*restrict dest_count,
                            const byte*restrict source,
                            const size_t source_size)
{
    FPZ* fpz = fpzip_read_from_buffer(dest);

    int ret = decompress(fpz, (void*)source, source_size);

    fpzip_read_close(fpz);

    return ret;
}

scil_compression_algorithm algo_fpzip = {
    .c.Dtype = {
        scil_fpzip_compress,
        scil_fpzip_decompress
    },
    "fpzip",
    4,
    SCIL_COMPRESSOR_TYPE_BASE_DATATYPE
};
