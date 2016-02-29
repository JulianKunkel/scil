#include <algo/gzip.h>

#include <zlib.h>

int scil_gzip_compress(const scil_context* ctx, byte* restrict dest, size_t* restrict dest_size, const double*restrict source, const size_t source_count){

    uLongf d_size = compressBound(source_count * sizeof(double));

    int ret = compress( (Bytef*)dest, &d_size, (Bytef*)source, (uLong)(source_count * sizeof(double)) );

    *dest_size = d_size;
    return ret;
}

int scil_gzip_decompress(const scil_context* ctx, double*restrict dest, size_t*restrict dest_count, const byte*restrict source, const size_t source_size){

    uLongf dest_size = *dest_count * sizeof(double);
    int ret = uncompress( dest, &dest_size, source, source_size);
    *dest_count = dest_size / sizeof(double);

    return ret;
}

scil_compression_algorithm algo_gzip = {
    scil_gzip_compress,
    scil_gzip_decompress,
    "gzip",
    2
};
