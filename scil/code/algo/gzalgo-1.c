// This file is part of SCIL.
// 
// SCIL is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// SCIL is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with SCIL.  If not, see <http://www.gnu.org/licenses/>.
#include <zlib.h>
#include <string.h>

#include <scil-util.h>

#include <algo/gzalgo-1.h>
#include <algo/algo-1.h>
#include <algo/gzip.h>

int scil_gzalgo1_compress(const scil_context* ctx, byte* restrict dest, size_t*restrict dest_size, const double*restrict source, const size_t source_count){

    int ret = 0;

    size_t inter_size = source_count * sizeof(double); // Ask Julian
    byte* intermed = (byte*)SAFE_MALLOC(inter_size);

    // Algo-1 compression
    ret = scil_algo1_compress(ctx, intermed, &inter_size, source, source_count);
    if(ret){
        fprintf(stderr, "Error in algo-1 partial compression of gzalgo-1. (Error code %d)\n", ret);
        return ret;
    }
    printf("Inter size: %lu\n", inter_size);

    // Gzip compression
    uLongf d_size = compressBound(inter_size);
    ret = compress( dest, &d_size, intermed, inter_size );
    if(ret){
        fprintf(stderr, "Error in gzip partial compression of gzalgo-1. (Error code %d)\n", ret);
        return ret;
    }
    *dest_size = d_size;

    free(intermed);

    return 0;
}

int scil_gzalgo1_decompress(const scil_context* ctx, double*restrict dest, size_t*restrict dest_count, const byte*restrict source, const size_t source_size){

    uLongf inter_size = *dest_count * sizeof(double);
    Bytef* intermed = (Bytef*)SAFE_MALLOC(inter_size);

    printf("inter_size:%d\n", inter_size);

    int ret = uncompress( intermed, &inter_size, (Bytef*)source, (uLongf)source_size);
    if(ret){
        fprintf(stderr, "Error in gzip partial decompression of gzalgo-1. (Error code %d)\n", ret);
    }

    printf("inter_size:%d\n", inter_size);

    ret = scil_algo1_decompress(ctx, dest, dest_count, intermed, inter_size);
    if(ret){
        fprintf(stderr, "Error in algo-1 partial decompression of gzalgo-1. (Error code %d)\n", ret);
    }

    return 0;

    /*
    int ret = 0;

    uLongf inter_size = *dest_count * sizeof(double);
    Bytef* intermed = (byte*)SAFE_MALLOC(inter_size);

    // Gzip decompression
    ret = uncompress( intermed, &inter_size, (Bytef*)source, (uLongf)source_size );
    if(ret){
        fprintf(stderr, "Error in gzip partial decompression of gzalgo-1. (Error code %d)\n", ret);
        for(size_t i = 0; i < *dest_count; ++i){
            dest[i] = 0.0;
        }
        return ret;
    }

    // Algo-1 decompression
    ret = scil_algo1_decompress(ctx, dest, dest_count, intermed, inter_size);
    if(ret){
        fprintf(stderr, "Error in algo-1 partial decompression of gzalgo-1. (Error code %d)\n", ret);
        for(size_t i = 0; i < *dest_count; ++i){
            dest[i] = 0.0;
        }
        return ret;
    }
    printf("Inter size: %lu\n", inter_size);

    free(intermed);

    return 0;*/
}


scil_compression_algorithm algo_gzalgo1 = {
    scil_gzalgo1_compress,
    scil_gzalgo1_decompress,
    "gzalgo-1",
    3
};
