#include <algo/gzip.h>

#include <zlib.h>

#include <util.h>

int scil_gzip_compress(const scil_context* ctx, byte* restrict dest, size_t* restrict dest_size, const double*restrict source, const size_t source_count){

    /*
    z_stream stream;
    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;

    stream.next_in = (Bytef*)source;
    stream.avail_in = source_count * sizeof(double);
    stream.next_out = (Bytef*)dest;
    stream.avail_out  = dest_size;

    deflateInit(&stream, Z_BEST_COMPRESSION);
    deflate(&stream, Z_FINISH);
    deflateEnd(&stream);
    */

    uLongf d_size = compressBound(source_count * sizeof(double));

    int ret = compress( (Bytef*)dest, &d_size, (Bytef*)source, (uLong)(source_count * sizeof(double)) );

    *dest_size = d_size;
    return ret;
}

int scil_gzip_decompress(const scil_context* ctx, double*restrict dest, size_t*restrict dest_count, const byte*restrict source, const size_t source_size){

    /*
    z_stream stream;
    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;

    stream.next_in = (Bytef*)source;
    stream.avail_in = source_size;
    stream.next_out = (Bytef*)dest;
    stream.avail_out  = dest_count;

    inflateInit(&stream);
    inflate(&stream, Z_NO_FLUSH);
    inflateEnd(&stream);
    */
    uLongf dest_size = *dest_count * sizeof(double);
    int ret = uncompress( (Bytef*)dest, &dest_size, (Bytef*)source, (uLong)source_size);

    return ret;
}
