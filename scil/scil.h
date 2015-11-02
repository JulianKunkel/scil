#ifndef SCIL_HEADER_
#define SCIL_HEADER_

typedef struct{
  // relative tolerance e.g. 1 means the value 2 can become 1-3
  double relative_tolerance;
  // 1 means 1% tolerance
  double absolute_tolerance_percent;
} scil_hints;


// TODO later: hide the structure in an internal header
typedef struct{
  scil_hints hints;
  //...
} scil_context;


int scil_create_compression_context(scil_context * out_ctx, scil_hints * hints);

int scil_compress(scil_context * ctx, char* compressed_buf, size_t* out_size, double * data, size_t count);

int scil_decompress(scil_context * ctx, double * data_out, size_t * out_size, char * compressed_buf);

#endif
