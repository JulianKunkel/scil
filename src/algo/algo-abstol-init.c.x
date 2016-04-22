#ifndef SCIL_ABSTOL_INITIALZIER
#define SCIL_ABSTOL_INITIALZIER

#define SCIL_SMALL 0.00000000000000000000001

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

scil_compression_algorithm algo_abstol = {
    .c.DNtype = {
        scil_abstol_compress_float,
        scil_abstol_decompress_float,
        scil_abstol_compress_double,
        scil_abstol_decompress_double,
    },
    "abstol",
    1,
    SCIL_COMPRESSOR_TYPE_DATATYPES
};

#endif
