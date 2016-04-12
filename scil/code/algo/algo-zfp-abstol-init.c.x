#ifndef SCIL_ZFP_ABSTOL_INITIALZIER
#define SCIL_ZFP_ABSTOL_INITIALZIER

scil_compression_algorithm algo_zfp_abstol = {
    .c.DNtype = {
        scil_zfp_abstol_compress_float,
        scil_zfp_abstol_decompress_float,
        scil_zfp_abstol_compress_double,
        scil_zfp_abstol_decompress_double,
    },
    "zfp-abstol",
    5,
    SCIL_COMPRESSOR_TYPE_DATATYPES
};

#endif
