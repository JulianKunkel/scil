#ifndef SCIL_HEADER_FPZIP_INITIALIZER_
#define SCIL_HEADER_FPZIP_INITIALIZER_

scil_compression_algorithm algo_fpzip = {
    .c.D1type = {
        scil_fpzip_compress_float,
        scil_fpzip_decompress_float,
        scil_fpzip_compress_double,
        scil_fpzip_decompress_double
    },
    "fpzip",
    4,
    SCIL_COMPRESSOR_TYPE_1D
};



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
  uint64_t outbytes = fpzip_write(fpz, data);
  if (!outbytes) {
    fprintf(stderr, "Compression failed in algo-fpzip compression: %s\n", fpzip_errstr[fpzip_errno]);
    return 1;
  }

  return 0;
}

// decompress 'inbytes' bytes to 'data' from stream 'FPZ'
static int
decompress(FPZ* fpz, void* data, uint64_t inbytes)
{
    // fpzip_read_header returns 1 on success and 0 on failure
  if (!fpzip_read_header(fpz)) {
    fprintf(stderr, "Cannot read header in algo-fpzip decompression: %s\n", fpzip_errstr[fpzip_errno]);
    return 1;
  }

  // make sure array size stored in header matches expectations
  uint64_t size = (fpz->type == FPZIP_TYPE_FLOAT ? sizeof(float) : sizeof(double));
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

#endif
