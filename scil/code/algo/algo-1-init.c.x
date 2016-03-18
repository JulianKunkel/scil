#ifndef SCIL_ALGO1_INITIALZIER
#define SCIL_ALGO1_INITIALZIER

scil_compression_algorithm algo_algo1 = {
    .c.DNtype = {
        scil_algo1_compress_float,
        scil_algo1_decompress_float,
        scil_algo1_compress_double,
        scil_algo1_decompress_double,
    },
    "algo1",
    1,
    SCIL_COMPRESSOR_TYPE_DATATYPES
};

#endif
