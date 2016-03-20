#ifndef SCIL_ZFP_INITIALZIER
#define SCIL_ZFP_INITIALZIER

scil_compression_algorithm algo_zfp = {
    .c.DNtype = {
        scil_zfp_compress_float,
        scil_zfp_decompress_float,
        scil_zfp_compress_double,
        scil_zfp_decompress_double,
    },
    "zfp",
    5,
    SCIL_COMPRESSOR_TYPE_DATATYPES
};

#endif
