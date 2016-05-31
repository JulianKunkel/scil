#ifndef SCIL_OPJ_INITIALZIER
#define SCIL_OPJ_INITIALZIER

static uint64_t round_up_byte(const uint64_t bits){

    uint8_t a = bits % 8;
    if(a == 0)
        return bits / 8;
    return 1 + (bits - a) / 8;
}

static uint8_t get_bits(const uint64_t num, const uint8_t start, const uint8_t size){

    assert(start <= 64);
    assert(size <= 8);

    return (uint8_t)((num << (64 - start)) >> (64 - size));
}

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
