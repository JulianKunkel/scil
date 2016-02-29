#ifndef SCIL_INTERNAL_HEADER_
#define SCIL_INTERNAL_HEADER_

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#include <scil.h>


typedef struct{
    int (*compress)(const scil_context* ctx, byte* restrict compressed_buf_out, size_t* restrict out_size, const double*restrict data_in, const size_t in_size);
    int (*decompress)(const scil_context* ctx, double*restrict data_out, size_t*restrict out_size, const byte*restrict compressed_buf_in, const size_t in_size);
    const char * name;
    int magic_number;
} scil_compression_algorithm;


struct scil_context_t{
  scil_hints hints;

  // the last compressor used, could be used for debugging
  scil_compression_algorithm * last_algorithm;

  // bla bla
};


#endif
