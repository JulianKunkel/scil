#ifndef SCIL_ALGO_MEMCOPY_H_
#define SCIL_ALGO_MEMCOPY_H_

#include <scil-internal.h>

int scil_memcopy_compress(const scil_context* ctx, char* restrict compressed_buf_out, size_t* restrict out_size, const double*restrict data_in, const size_t in_size);
int scil_memcopy_decompress(const scil_context* ctx, double*restrict data_out, size_t*restrict out_size, const char*restrict compressed_buf_in, const size_t in_size);

scil_compression_algorithm algo_memcopy = {
    scil_memcopy_compress,
    scil_memcopy_decompress,
    "memcopy",
    0
};

#endif
