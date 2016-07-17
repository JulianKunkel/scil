#ifndef SCIL_OPJ_INITIALZIER
#define SCIL_OPJ_INITIALZIER

scil_compression_algorithm algo_opj = {
    .c.DNtype = {
	      scil_opj_compress_float,
        scil_opj_decompress_float,
        scil_opj_compress_double,
        scil_opj_decompress_double,
    },
    "opj",
    9,//magic_number
    SCIL_COMPRESSOR_TYPE_DATATYPES,
    0 //is_lossy
};


#endif
