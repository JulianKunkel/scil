#ifndef SCIL_ALGO1_INITIALZIER
#define SCIL_ALGO1_INITIALZIER

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
