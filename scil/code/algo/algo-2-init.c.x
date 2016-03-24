#ifndef SCIL_ALGO_2_INITIALIZER
#define SCIL_ALGO_2_INITIALIZER

#include <scil-internal.h>

#include <math.h>

static uint64_t mask[] = {
    0,
    1,
    3,
    7,
    15,
    31,
    63,
    127,
    255,
    511,
    1023,
    2047,
    4095,
    8191,
    16383,
    32767,
    65535,
    131071,
    262143,
    524287,
    1048575,
    2097151,
    4194303,
    8388607,
    16777215,
    33554431,
    67108863,
    134217727,
    268435455,
    536870911,
    1073741823,
    2147483647,
    4294967295,
    8589934591,
    17179869183,
    34359738367,
    68719476735,
    137438953471,
    274877906943,
    549755813887,
    1099511627775,
    2199023255551,
    4398046511103,
    8796093022207,
    17592186044415,
    35184372088831,
    70368744177663,
    140737488355327,
    281474976710655,
    562949953421311,
    1125899906842623,
    2251799813685247,
    4503599627370495,
};

static uint8_t calc_sign_bits(uint8_t min_sign, uint8_t max_sign){

    return min_sign == max_sign ? min_sign : 2;
}

static uint8_t calc_exp_bits(uint16_t min_exponent, uint16_t max_exponent){

    return (uint8_t)ceil(log2(max_exponent - min_exponent));
}

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

static uint8_t get_sign(uint64_t value, uint8_t bits_per_num, uint8_t signs_id){

    if(signs_id != 2) return signs_id;

    return value >> (bits_per_num - 1);
}

static int16_t get_exponent(uint64_t value, uint8_t exp_bits, uint8_t mant_bits, int16_t min_exponent){

    return min_exponent + ((value & (mask[mant_bits + exp_bits] ^ mask[mant_bits])) >> mant_bits);
}

static uint32_t get_mantisa_float(uint64_t value, uint8_t mant_bits){

    return (value & mask[mant_bits]) << (MANTISA_LENGTH_float - mant_bits);
}

static uint64_t get_mantisa_double(uint64_t value, uint8_t mant_bits){

    return (value & mask[mant_bits]) << (MANTISA_LENGTH_double - mant_bits);
}

scil_compression_algorithm algo_algo2 = {
    .c.DNtype = {
        scil_algo2_compress_float,
        scil_algo2_decompress_float,
        scil_algo2_compress_double,
        scil_algo2_decompress_double,
    },
    "algo2",
    3,
    SCIL_COMPRESSOR_TYPE_DATATYPES
};

#endif
