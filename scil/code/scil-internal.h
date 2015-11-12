#ifndef SCIL_INTERNAL_HEADER_
#define SCIL_INTERNAL_HEADER_

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#include "scil.h"


struct scil_context_t{
  scil_hints hints;
  // bla bla
};

// known algorithms:

int scil_algo1_compress(const scil_context* ctx, char** restrict compressed_buf_out, size_t* restrict out_size, const double*restrict data_in, const size_t in_size);
int scil_algo1_decompress(const scil_context* ctx, double*restrict data_out, const size_t*restrict out_size, const char*restrict compressed_buf_in, const size_t in_size);


#endif 