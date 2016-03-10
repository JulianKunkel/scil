#include <algo/algo-1.h>

scil_compression_algorithm algo_algo1 = {
    .c.D1type = {
        scil_algo1_compress_float,
        scil_algo1_decompress_float,
        scil_algo1_compress_double,
        scil_algo1_decompress_double,
    },
    "algo1",
    1,
    SCIL_COMPRESSOR_TYPE_1D
};
