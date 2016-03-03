#include <algo/gzip.h>

#include <zlib.h>

int scil_gzip_compress(const scil_context* ctx, byte* restrict dest, size_t* restrict dest_size, const double*restrict source, const size_t source_count){

    return compress( (Bytef*)dest, dest_size, (Bytef*)source, (uLong)(source_count * sizeof(double)) );
}

int scil_gzip_decompress(const scil_context* ctx, double*restrict dest, size_t*restrict dest_count, const byte*restrict source, const size_t source_size){

    uLongf dest_size = *dest_count * sizeof(double);
    int ret = uncompress( (Bytef*)dest, &dest_size, (Bytef*)source, (uLongf)source_size);

    if(ret){
        fprintf(stderr, "Error in gzip decompression. (Error code %d)\n", ret);
    }

    *dest_count = dest_size / sizeof(double);

    return ret;
}

scil_compression_algorithm algo_gzip = {
    scil_gzip_compress,
    scil_gzip_decompress,
    "gzip",
    2
};
