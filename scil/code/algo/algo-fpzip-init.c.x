#ifndef SCIL_FPZIP_INITIALIZER
#define SCIL_FPZIP_INITIALIZER

// compress 'inbytes' bytes from 'data' to stream 'FPZ'
static int compress(FPZ* fpz, void* data)
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

// decompress bytes to 'data' from stream 'FPZ'
static int decompress(FPZ* fpz, void* data)
{
    // fpzip_read_header returns 1 on success and 0 on failure
  if (!fpzip_read_header(fpz)) {
    fprintf(stderr, "Cannot read header in algo-fpzip decompression: %s\n", fpzip_errstr[fpzip_errno]);
    return 1;
  }

  // perform actual decompression
  if (!fpzip_read(fpz, data)) {
    fprintf(stderr, "Algo-fpzip decompression failed: %s\n", fpzip_errstr[fpzip_errno]);
    return 1;
  }

  return 0;
}

scil_compression_algorithm algo_fpzip = {
    .c.DNtype = {
        scil_fpzip_compress_float,
        scil_fpzip_decompress_float,
        scil_fpzip_compress_double,
        scil_fpzip_decompress_double
    },
    "fpzip",
    4,
    SCIL_COMPRESSOR_TYPE_DATATYPES
};

#endif
