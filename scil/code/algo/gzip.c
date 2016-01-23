#include <algo/gzip.h>

#include <zlib.h>

#include <util.h>

int scil_gzip_compress(const scil_context* ctx, char* restrict compressed_buf_out, size_t* restrict out_size, const double*restrict data_in, const size_t in_size){

    return compress(compressed_buf_out, out_size, data_in, in_size);
}

int scil_gzip_decompress(const scil_context* ctx, double*restrict data_out, size_t*restrict out_size, const char*restrict compressed_buf_in, const size_t in_size){

    return uncompress(data_out, out_size, compressed_buf_in, in_size);
}
