#ifndef SCIL_ALGO_MEMCOPY_H_
#define SCIL_ALGO_MEMCOPY_H_

#include <scil-internal.h>

int scil_memcopy_compress(const scil_context* ctx, byte* restrict dest, size_t*restrict dest_size, const double*restrict source, const size_t source_count);
int scil_memcopy_decompress(const scil_context* ctx, double*restrict dest, size_t*restrict dest_count, const byte*restrict source, const size_t source_size);

extern scil_compression_algorithm algo_memcopy;

#endif
