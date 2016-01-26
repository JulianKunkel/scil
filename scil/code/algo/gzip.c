#include <algo/gzip.h>

#include <zlib.h>

#include <util.h>

int scil_gzip_compress(const scil_context* ctx, byte* restrict dest, size_t* restrict dest_size, const double*restrict source, const size_t source_count){

    return compress(dest, dest_size, source, source_count);
}

int scil_gzip_decompress(const scil_context* ctx, double*restrict dest, size_t*restrict dest_count, const byte*restrict source, const size_t source_size){

    return uncompress(dest, dest_count, source, source_size);
}
