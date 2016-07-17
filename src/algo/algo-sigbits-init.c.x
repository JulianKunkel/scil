#ifndef SCIL_ALGO_2_INITIALIZER
#define SCIL_ALGO_2_INITIALIZER

#include <scil-internal.h>

#include <math.h>

#define SCIL_SIGBITS_HEADER_SIZE 5

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

static void read_header(const byte* source,
                        size_t* const source_size,
                        uint8_t* const signs_id,
                        uint8_t* const exponent_bit_count,
                        uint8_t* const mantissa_bit_count,
                        int16_t* const minimum_exponent){

    *signs_id = *((uint8_t*)source);
    source += 1;
    *source_size -= 1;

    *exponent_bit_count = *((uint8_t*)source);
    source += 1;
    *source_size -= 1;

    *mantissa_bit_count = *((uint8_t*)source);
    source += 1;
    *source_size -= 1;

    *minimum_exponent = *((int16_t*)source);
    source += 2;
    *source_size -= 2;
}

static void write_header(byte* dest,
                         uint8_t signs_id,
                         uint8_t exponent_bit_count,
                         uint8_t mantissa_bit_count,
                         uint16_t minimum_exponent){

    *dest = signs_id;
    ++dest;

    *dest = exponent_bit_count;
    ++dest;

    *dest = mantissa_bit_count;
    ++dest;

    *((int16_t*)dest) = minimum_exponent;
    dest += 2;
}

static uint8_t calc_sign_bit_count(uint8_t minimum_sign, uint8_t maximum_sign){

    return minimum_sign == maximum_sign ? minimum_sign : 2;
}

static uint8_t calc_exponent_bit_count(int16_t minimum_exponent, int16_t max_exponent){

    return (uint8_t)ceil(log2(max_exponent - minimum_exponent));
}

static uint64_t round_up_byte(const uint64_t bits){

    uint8_t a = bits % 8;
    if(a == 0)
        return bits / 8;
    return 1 + (bits - a) / 8;
}

static uint8_t get_bit_count_per_value(uint8_t signs_id, uint8_t exponent_bit_count, uint8_t mantissa_bit_count){

    return (signs_id == 2) + exponent_bit_count + mantissa_bit_count;
}

static uint8_t get_sign(uint64_t value, uint8_t bits_per_value, uint8_t signs_id){

    if(signs_id != 2) return signs_id;

    return value >> (bits_per_value - 1);
}

static int16_t get_exponent(uint64_t value, uint8_t exponent_bit_count, uint8_t mantissa_bit_count, int16_t minimum_exponent){

    return minimum_exponent + ((value & (mask[mantissa_bit_count + exponent_bit_count] ^ mask[mantissa_bit_count])) >> mantissa_bit_count);
}

static uint32_t get_mantissa_float(uint64_t value, uint8_t mantissa_bit_count){

    return (value & mask[mantissa_bit_count]) << (MANTISSA_LENGTH_FLOAT - mantissa_bit_count);
}

static uint64_t get_mantissa_double(uint64_t value, uint8_t mantissa_bit_count){

    return (value & mask[mantissa_bit_count]) << (MANTISSA_LENGTH_DOUBLE - mantissa_bit_count);
}

scil_compression_algorithm algo_sigbits = {
    .c.DNtype = {
        scil_sigbits_compress_float,
        scil_sigbits_decompress_float,
        scil_sigbits_compress_double,
        scil_sigbits_decompress_double,
    },
    "sigbits",
    3,
    SCIL_COMPRESSOR_TYPE_DATATYPES,
    1
};

#endif
