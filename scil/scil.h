#ifndef SCIL_HEADER_
#define SCIL_HEADER_

#include <stdio.h>

typedef struct{
  // 1 means 1% tolerance
  double relative_tolerance_percent;
  // relative tolerance e.g. 1 means the value 2 can become 1-3
  double absolute_tolerance;
} scil_hints;


// TODO later: hide the structure in an internal header
typedef struct{
  scil_hints hints;
  //...
} scil_context;


int scil_create_compression_context(scil_context * out_ctx, scil_hints * hints);

int scil_compress(scil_context* ctx, char* compressed_buf_out, size_t* out_size, const double* data_in, const size_t in_size);

int scil_decompress(scil_context* ctx, double* data_out, size_t* out_size, const char* compressed_buf_in, const size_t in_size);

#endif
