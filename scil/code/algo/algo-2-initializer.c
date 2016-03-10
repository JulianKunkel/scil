#include <algo/algo-2.h>

scil_compression_algorithm algo_algo2 = {
    .c.D1type = {
        scil_algo2_compress_float,
        scil_algo2_decompress_float,
        scil_algo2_compress_double,
        scil_algo2_decompress_double,
    },
    "algo2",
    3,
    SCIL_COMPRESSOR_TYPE_1D
};
