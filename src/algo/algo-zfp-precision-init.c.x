#ifndef SCIL_ZFP_PRECISION_INITIALZIER
#define SCIL_ZFP_PRECISION_INITIALZIER

scil_compression_algorithm algo_zfp_precision = {
    .c.DNtype = {
        scil_zfp_precision_compress_float,
        scil_zfp_precision_decompress_float,
        scil_zfp_precision_compress_double,
        scil_zfp_precision_decompress_double,
    },
    "zfp-precision",
    6,
    SCIL_COMPRESSOR_TYPE_DATATYPES,
    1
};

#endif
