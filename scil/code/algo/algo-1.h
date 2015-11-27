#ifndef SCIL_ALGO1_H_
#define SCIL_ALGO1_H_

#include <scil-internal.h>

int scil_algo1_compress(const scil_context* ctx, char** restrict compressed_buf_out, size_t* restrict out_size, const double*restrict data_in, const size_t in_size);
int scil_algo1_decompress(const scil_context* ctx, double*restrict data_out, size_t*restrict out_size, const char*restrict compressed_buf_in, const size_t in_size);

scil_compression_algorithm algo_algo1 = {
    scil_algo1_compress,
    scil_algo1_decompress,
    "algo1",
    2
};

#endif
