#include <algo/memcopy.h>

#include <string.h>

int scil_memcopy_compress(const scil_context* ctx, char** restrict compressed_buf_out, size_t* restrict out_size, const double*restrict data_in, const size_t in_size){
    // TODO check if out_size is sufficently large
    *out_size = in_size;
    return memcpy(compressed_buf_out, data_in, in_size);
}

int scil_memcopy_decompress(const scil_context* ctx, double*restrict data_out, size_t*restrict out_size, const char*restrict compressed_buf_in, const size_t in_size){
    // TODO chekc if buff is sufficiently large
    *out_size = in_size;
    return memcpy(data_out, compressed_buf_in, in_size);
}
 #warning USING INTERNAL HEADER

