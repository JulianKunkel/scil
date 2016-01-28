#include <algo/gzalgo-1.h>

#include <zlib.h>

#include <util.h>
#include <algo/algo-1.h>
#include <algo/gzip.h>

int scil_gzalgo1_compress(const scil_context* ctx, byte* restrict dest, size_t*restrict dest_size, const double*restrict source, const size_t source_count){

    int ret = 0;

    size_t inter_size = source_count * sizeof(double); // Ask Julian
    byte* intermed = (byte*)SAFE_MALLOC(inter_size);

    // Algo-1 compression
    ret = scil_algo1_compress(ctx, intermed, &inter_size, source, source_count);
    if(ret){
        fprintf(stderr, "Error in algo-1 partial compression of gzalgo-1\n");
        return ret;
    }
    printf("Inter size: %lu\n", inter_size);

    // Gzip compression
    uLongf d_size = compressBound(inter_size);
    ret = compress( dest, &d_size, intermed, inter_size );
    if(ret){
        fprintf(stderr, "Error in gzip partial compression of gzalgo-1\n");
        return ret;
    }
    *dest_size = d_size;

    free(intermed);

    return 0;
}

int scil_gzalgo1_decompress(const scil_context* ctx, double*restrict dest, size_t*restrict dest_count, const byte*restrict source, const size_t source_size){

    int ret = 0;

    size_t inter_size = source_count * sizeof(double); // Ask Julian
    byte* intermed = (byte*)SAFE_MALLOC(inter_size);

    // Gzip decompression
    uLongf d_size = compressBound(inter_size);
    ret = uncompress( intermed, &inter_size, source, source_size );
    if(ret){
        fprintf(stderr, "Error in gzip partial decompression of gzalgo-1\n");
        return ret;
    }
    *dest_size = d_size;

    // Algo-1 compression
    ret = scil_algo1_compress(ctx, intermed, &inter_size, source, source_count);
    if(ret){
        fprintf(stderr, "Error in algo-1 partial decompression of gzalgo-1\n");
        return ret;
    }
    printf("Inter size: %lu\n", inter_size);

    *dest_count =

    free(intermed);

    return 0;
}

scil_compression_algorithm algo_gzalgo1 = {
    scil_gzalgo1_compress,
    scil_gzalgo1_decompress,
    "gzalgo-1",
    3
};
