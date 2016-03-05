#include <algo/memcopy.h>

#include <string.h>

int scil_memcopy_compress(const scil_context* ctx, byte* restrict dest, size_t*restrict dest_size, const double*restrict source, const size_t source_count){
    // TODO check if out_size is sufficently large
    *dest_size = source_count;
    return memcpy(dest, source, source_count);
}

int scil_memcopy_decompress(const scil_context* ctx, double*restrict dest, size_t*restrict dest_count, const byte*restrict source, const size_t source_size){
    // TODO check if buff is sufficiently large
    *dest_count = source_size;
    return memcpy(dest, source, source_size);
}

scil_compression_algorithm algo_memcopy = {
    scil_memcopy_compress,
    scil_memcopy_decompress,
    "memcopy",
    0
};
