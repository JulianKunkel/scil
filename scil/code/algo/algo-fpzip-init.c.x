#ifndef SCIL_FPZIP_INITIALIZER
#define SCIL_FPZIP_INITIALIZER

scil_compression_algorithm algo_fpzip = {
    .c.DNtype = {
        scil_fpzip_compress_float,
        scil_fpzip_decompress_float,
        scil_fpzip_compress_double,
        scil_fpzip_decompress_double
    },
    "fpzip",
    4,
    SCIL_COMPRESSOR_TYPE_DATATYPES
};

#endif
