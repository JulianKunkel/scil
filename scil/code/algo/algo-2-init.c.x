#ifndef SCIL_ALGO_2_INITIALIZER
#define SCIL_ALGO_2_INITIALIZER

#include <math.h>

/*
#define MANTISSA_MAX_LENGTH 52

#define MANTISA_LENGTH_float 23

typedef union {
  struct {
    uint32_t mantisa  : MANTISA_LENGTH_float;
    uint32_t exponent : 8;
    uint32_t sign     : 1;
  } p;
	float f;
} datatype_cast_float;

#define MANTISA_LENGTH_double 52

typedef union {
  struct {
    uint64_t mantisa  : MANTISA_LENGTH_double;
    uint32_t exponent : 11;
    uint32_t sign     : 1;
  } p;
	double f;
} datatype_cast_double;
*/

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

    uint8_t pos = (bits_per_num - 1);
    uint64_t mask = 1 << pos;
    return (value & mask) >> pos;
}

static int16_t get_exponent(uint64_t value, uint8_t exp_bits, uint8_t mant_bits, int16_t min_exponent){

    uint64_t mask = (1 << (mant_bits + exp_bits)) - 1; // i.e 00001111

    uint64_t mask_mask = (1 << mant_bits) - 1;

    mask ^= mask_mask;
    return min_exponent + ((value & mask) >> mant_bits);
}

static uint64_t get_mantisa(uint64_t value, uint8_t mant_bits){

    uint64_t mask = (1 << mant_bits) - 1;

    return value & mask;
}

static double decompress_double(uint64_t value, uint8_t bits_per_num, uint8_t signs_id, uint8_t exp_bits, uint8_t mant_bits, int16_t min_exponent){

    datatype_cast_double cur;

    cur.p.sign = get_sign(value, bits_per_num, signs_id);
    cur.p.exponent = get_exponent(value, exp_bits, mant_bits, min_exponent);
    cur.p.mantisa = get_mantisa(value, mant_bits);

    return cur.f;
}

static double decompress_float(uint64_t value, uint8_t bits_per_num, uint8_t signs_id, uint8_t exp_bits, uint8_t mant_bits, int16_t min_exponent){

    datatype_cast_double cur;

    cur.p.sign = get_sign(value, bits_per_num, signs_id);
    cur.p.exponent = get_exponent(value, exp_bits, mant_bits, min_exponent);
    cur.p.mantisa = (uint32_t)get_mantisa(value, mant_bits);

    return cur.f;
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
