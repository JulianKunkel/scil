// This file is part of SCIL.
//
// SCIL is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SCIL is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with SCIL.  If not, see <http://www.gnu.org/licenses/>.

#include <algo/algo-sigbits.h>

#include <scil.h>

#include <scil-error.h>
#include <scil-internal.h>
#include <scil-swager.h>
#include <scil-util.h>

#include <math.h>

#define SCIL_SIGBITS_HEADER_SIZE 13

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
                        size_t* source_size,
                        uint8_t* signs_id,
                        uint8_t* exponent_bit_count,
                        uint8_t* mantissa_bit_count,
                        int16_t* minimum_exponent,
                        double *fill_value,
                        uint8_t *fill_value_exponent){

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

    scilU_unpack8(source, fill_value);
    source += 8;
    *source_size -= 8;

    if(*fill_value != DBL_MAX){
      *fill_value_exponent = *((int8_t*)source);
      source += 1;
      *source_size -= 1;
    }
}

static void write_header(byte* dest,
                         uint8_t signs_id,
                         uint8_t exponent_bit_count,
                         uint8_t mantissa_bit_count,
                         uint16_t minimum_exponent,
                         double fill_value,
                         uint8_t fill_value_exponent){

    *dest = signs_id;
    ++dest;

    *dest = exponent_bit_count;
    ++dest;

    *dest = mantissa_bit_count;
    ++dest;

    *((int16_t*)dest) = minimum_exponent;
    dest += 2;

    scilU_pack8(dest, fill_value);
    dest += 8;

    if (fill_value != DBL_MAX){
      *dest = fill_value_exponent;
      ++dest;
    }
}

static uint8_t calc_sign_bit_count(uint8_t minimum_sign, uint8_t maximum_sign){

    return minimum_sign == maximum_sign ? minimum_sign : 2;
}

static uint8_t calc_exponent_bit_count(int16_t minimum_exponent, int16_t max_exponent){
    return (uint8_t)ceil(log2(max_exponent - minimum_exponent + 1));
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

//Supported datatypes: double float
// Repeat for each data type

static void find_minimums_and_maximums_<DATATYPE>(const <DATATYPE>* buffer,
                                                  const size_t size,
                                                  uint8_t* minimum_sign,
                                                  uint8_t* maximum_sign,
                                                  int16_t* minimum_exponent,
                                                  int16_t* maximum_exponent){

    *minimum_sign = 1;
    *maximum_sign = 0;

    *minimum_exponent = 0x7fff;
    *maximum_exponent = -*minimum_exponent;

    for(size_t i = 0; i < size; ++i){

        datatype_cast_<DATATYPE> cur;
        cur.f = buffer[i];

        if(*minimum_sign != 0 && cur.p.sign < *minimum_sign) { *minimum_sign = cur.p.sign; }
        if(*maximum_sign != 1 && cur.p.sign > *maximum_sign) { *maximum_sign = cur.p.sign; }

        if(cur.p.exponent < *minimum_exponent) { *minimum_exponent = cur.p.exponent; }
        if(cur.p.exponent > *maximum_exponent) { *maximum_exponent = cur.p.exponent; }
    }
}

static void find_minimums_and_maximums_fill_<DATATYPE>(const <DATATYPE>* buffer,
                                                  const size_t size,
                                                  uint8_t* minimum_sign,
                                                  uint8_t* maximum_sign,
                                                  int16_t* minimum_exponent,
                                                  int16_t* maximum_exponent,
                                                  double fill_value){

    *minimum_sign = 1;
    *maximum_sign = 0;

    *minimum_exponent = 0x7fff;
    *maximum_exponent = -*minimum_exponent;

    for(size_t i = 0; i < size; ++i){

        if (buffer[i] != fill_value)
        {
        datatype_cast_<DATATYPE> cur;
        cur.f = buffer[i];

        if(*minimum_sign != 0 && cur.p.sign < *minimum_sign) { *minimum_sign = cur.p.sign; }
        if(*maximum_sign != 1 && cur.p.sign > *maximum_sign) { *maximum_sign = cur.p.sign; }

        if(cur.p.exponent < *minimum_exponent) { *minimum_exponent = cur.p.exponent; }
        if(cur.p.exponent > *maximum_exponent) { *maximum_exponent = cur.p.exponent; }
      }
    }
}

static void get_header_data_<DATATYPE>(const <DATATYPE>* source,
                                       size_t count,
                                       uint8_t* signs_id,
                                       uint8_t* exponent_bit_count,
                                       int16_t* minimum_exponent){

    uint8_t minimum_sign, maximum_sign;
    int16_t maximum_exponent;
    find_minimums_and_maximums_<DATATYPE>(source,
                                          count,
                                          &minimum_sign,
                                          &maximum_sign,
                                          minimum_exponent,
                                          &maximum_exponent);

    *signs_id = calc_sign_bit_count(minimum_sign, maximum_sign);
    *exponent_bit_count = calc_exponent_bit_count(*minimum_exponent, maximum_exponent);
}

static void get_header_data_fill_<DATATYPE>(const <DATATYPE>* source,
                                       size_t count,
                                       uint8_t* signs_id,
                                       uint8_t* exponent_bit_count,
                                       int16_t* minimum_exponent,
                                       double fill_value){

    uint8_t minimum_sign, maximum_sign;
    int16_t maximum_exponent;
    find_minimums_and_maximums_fill_<DATATYPE>(source,
                                          count,
                                          &minimum_sign,
                                          &maximum_sign,
                                          minimum_exponent,
                                          &maximum_exponent,
                                          fill_value);

    *signs_id = calc_sign_bit_count(minimum_sign, maximum_sign);
    *exponent_bit_count = calc_exponent_bit_count(*minimum_exponent, maximum_exponent);
}

// TODO: Test this mess... Especially for the super rare exponent maximum overflow.
// TODO: Speed up shifts with lookup table.
static inline uint64_t compress_value_<DATATYPE>(<DATATYPE> value,
                                          uint8_t signs_id,
                                          uint8_t exponent_bit_count,
                                          uint8_t mantissa_bit_count,
                                          int16_t minimum_exponent){

    uint64_t result = 0;

    datatype_cast_<DATATYPE> cur;
    cur.f = value;

    // Calculate potentionally compressed sign bit, writing it and shifting to get space for exponent bits
    if(signs_id == 2){
        result = (uint64_t)cur.p.sign << exponent_bit_count;
    }

    // Amount of bitshifts to do at various points
    uint8_t shifts = MANTISSA_LENGTH_<DATATYPE_UPPER> - mantissa_bit_count;

    // Calculating compressed mantissa with rounding
    uint64_t inter_mantissa = cur.p.mantissa; // (cur.p.mantissa & (1UL << (shifts - 1)));

    // Calculating compressed exponent with potential overflow from mantissa due to rounding up and writing it
    result |= (uint64_t)(cur.p.exponent - minimum_exponent + (inter_mantissa > mask[52]));

    // Shifting to get space for mantissa
    result <<= mantissa_bit_count;

    // Clear overflow bit in mantissa
    //inter_mantissa &= ~(1 << shifts);
    // Write significant bits of mantissa
    result |= inter_mantissa >> shifts;

    /*
    // internal check for correctness:
    for(int m = 0; m < mantissa_bit_count; m++){
			int b1 = (cur.p.mantissa >> (MANTISSA_LENGTH_<DATATYPE_UPPER>-m)) & (1);
			int b2 = (inter_mantissa >> (MANTISSA_LENGTH_<DATATYPE_UPPER>-m)) & (1);
      printf("now: %d; %d = %d\n", m, b1, b2);
      assert(b1 == b2);
    }
    */

    return result;
}

static inline  <DATATYPE> decompress_value_<DATATYPE>(uint64_t value,
                                              uint8_t bit_count_per_value,
                                              uint8_t signs_id,
                                              uint8_t exponent_bit_count,
                                              uint8_t mantissa_bit_count,
                                              int16_t minimum_exponent){

    datatype_cast_<DATATYPE> cur;

    cur.p.sign     = get_sign(value, bit_count_per_value, signs_id);
    cur.p.exponent = get_exponent(value, exponent_bit_count, mantissa_bit_count, minimum_exponent);
    cur.p.mantissa = get_mantissa_<DATATYPE>(value, mantissa_bit_count);

    return cur.f;
}

static int compress_buffer_<DATATYPE>(uint64_t* restrict dest,
                                      const <DATATYPE>* restrict source,
                                      size_t count,
                                      uint8_t signs_id,
                                      uint8_t exponent_bit_count,
                                      uint8_t mantissa_bit_count,
                                      int16_t minimum_exponent){

    for(size_t i = 0; i < count; ++i){
        dest[i] = compress_value_<DATATYPE>(source[i], signs_id, exponent_bit_count, mantissa_bit_count, minimum_exponent);
    }

    return SCIL_NO_ERR;
}

static int compress_buffer_fill_<DATATYPE>(uint64_t* restrict dest,
                                      const <DATATYPE>* restrict source,
                                      size_t count,
                                      uint8_t signs_id,
                                      uint8_t exponent_bit_count,
                                      uint8_t mantissa_bit_count,
                                      int16_t minimum_exponent,
                                      double fill_value,
                                      uint8_t fill_value_exponent){

    for(size_t i = 0; i < count; ++i){
      if (source[i] != fill_value){
        dest[i] = compress_value_<DATATYPE>(source[i], signs_id, exponent_bit_count, mantissa_bit_count, minimum_exponent);
      }else{
        uint64_t result = 0;

        result |= fill_value_exponent;
        result <<= mantissa_bit_count;
        dest[i] = result;
      }
    }

    return SCIL_NO_ERR;
}

static int decompress_buffer_<DATATYPE>(<DATATYPE>* restrict dest,
                                        const uint64_t* restrict source,
                                        size_t count,
                                        uint8_t bit_count_per_value,
                                        uint8_t signs_id,
                                        uint8_t exponent_bit_count,
                                        uint8_t mantissa_bit_count,
                                        int16_t minimum_exponent){

    for (size_t i = 0; i < count; ++i) {
        dest[i] = decompress_value_<DATATYPE>(source[i], bit_count_per_value, signs_id, exponent_bit_count, mantissa_bit_count, minimum_exponent);
    }

    return SCIL_NO_ERR;
}

static int decompress_buffer_fill_<DATATYPE>(<DATATYPE>* restrict dest,
                                        const uint64_t* restrict source,
                                        size_t count,
                                        uint8_t bit_count_per_value,
                                        uint8_t signs_id,
                                        uint8_t exponent_bit_count,
                                        uint8_t mantissa_bit_count,
                                        int16_t minimum_exponent,
                                        double fill_value,
                                        uint8_t fill_value_exponent){

    uint64_t fill_mask = fill_value_exponent << (mantissa_bit_count-1);

    for(size_t i = 0; i < count; ++i){
      if (source[i] != fill_mask){
        dest[i] = decompress_value_<DATATYPE>(source[i], bit_count_per_value, signs_id, exponent_bit_count, mantissa_bit_count, minimum_exponent);
      }else{
        dest[i] = fill_value;
      }
    }

    return SCIL_NO_ERR;
}

int scil_sigbits_compress_<DATATYPE>(const scil_context_t* ctx,
                                     byte * restrict dest,
                                     size_t* dest_size,
                                     <DATATYPE>*restrict source,
                                     const scil_dims_t* dims){

    assert(ctx != NULL);
    assert(dest != NULL);
    assert(dest_size != NULL);
    assert(source != NULL);
    assert(dims != NULL);

    // ==================== Initialization =====================================

    int8_t mantissa_bit_count = ctx->hints.significant_bits - 1;
    // Check whether sigbit compression makes sense
    if(mantissa_bit_count == SCIL_ACCURACY_INT_FINEST){
        return SCIL_PRECISION_ERR;
    }

    size_t count = scilPr_get_dims_count(dims);

    uint8_t signs_id, exponent_bit_count;
    int16_t minimum_exponent;

    if (ctx->hints.fill_value == DBL_MAX){
      get_header_data_<DATATYPE>(source, count, &signs_id, &exponent_bit_count, &minimum_exponent);
    }else{ // use the fill value
      get_header_data_fill_<DATATYPE>(source, count, &signs_id, &exponent_bit_count, &minimum_exponent, ctx->hints.fill_value);
    }

    uint8_t bit_count_per_value = get_bit_count_per_value(signs_id, exponent_bit_count, mantissa_bit_count);

    uint8_t fill_value_exponent = 127;

    write_header(dest, signs_id, exponent_bit_count, mantissa_bit_count, minimum_exponent, ctx->hints.fill_value, fill_value_exponent);
    dest += SCIL_SIGBITS_HEADER_SIZE;

    *dest_size = round_up_byte(bit_count_per_value * count) + SCIL_SIGBITS_HEADER_SIZE;

    int ret = SCIL_NO_ERR;

    // ==================== Compression ========================================

    // Allocate intermediate buffer
    uint64_t* compressed_buffer = (uint64_t*)SAFE_MALLOC(count * sizeof(uint64_t));

    if (ctx->hints.fill_value == DBL_MAX){
      // Compress each value in source buffer
      if(compress_buffer_<DATATYPE>(compressed_buffer, source, count, signs_id, exponent_bit_count, mantissa_bit_count, minimum_exponent)){
        ret = SCIL_BUFFER_ERR;
        goto comp_cleanup;
      }
    }else{ // don't compress the fill value
      if(compress_buffer_fill_<DATATYPE>(compressed_buffer, source, count, signs_id, exponent_bit_count, mantissa_bit_count, minimum_exponent, ctx->hints.fill_value, fill_value_exponent)){
        ret = SCIL_BUFFER_ERR;
        goto comp_cleanup;
      }
    }

    // Pack compressed values tightly
    if(scil_swage(dest, compressed_buffer, count, (uint8_t)bit_count_per_value)){
        ret = SCIL_BUFFER_ERR;
        goto comp_cleanup;
    }

    // ==================== Cleanup ============================================

    comp_cleanup:
    free(compressed_buffer);
    return ret;
}

int scil_sigbits_decompress_<DATATYPE>(<DATATYPE>*restrict dest,
                                       scil_dims_t* dims,
                                       byte*restrict source,
                                       size_t source_size){

    assert(dest != NULL);
    assert(dims != NULL);
    assert(source != NULL);

    double fill_value = 0;

    // ==================== Initialization =====================================

    size_t count = scilPr_get_dims_count(dims);

    size_t source_size_cp = source_size;

    uint8_t signs_id, exponent_bit_count, mantissa_bit_count;
    int16_t minimum_exponent;
    uint8_t fill_value_exponent;
    read_header(source, &source_size_cp, &signs_id, &exponent_bit_count, &mantissa_bit_count, &minimum_exponent, &fill_value, &fill_value_exponent);
    source += SCIL_SIGBITS_HEADER_SIZE;

    uint8_t bit_count_per_value = get_bit_count_per_value(signs_id, exponent_bit_count, mantissa_bit_count);

    // ==================== Decompression ======================================

    uint64_t* unswaged_buffer = (uint64_t*)SAFE_MALLOC(count * sizeof(uint64_t));

    int ret = SCIL_NO_ERR;

    if(scil_unswage(unswaged_buffer, source, count, bit_count_per_value)){
        ret = SCIL_BUFFER_ERR;
        goto decomp_cleanup;
    }

    if (fill_value == DBL_MAX){
      // Deompress each value in source buffer
      if(decompress_buffer_<DATATYPE>(dest, unswaged_buffer, count, bit_count_per_value, signs_id, exponent_bit_count, mantissa_bit_count, minimum_exponent) ){
          ret = SCIL_BUFFER_ERR;
          goto decomp_cleanup;
      }
    }else{ // set fill value
      if(decompress_buffer_fill_<DATATYPE>(dest, unswaged_buffer, count, bit_count_per_value, signs_id, exponent_bit_count, mantissa_bit_count, minimum_exponent, fill_value, fill_value_exponent) ){
          ret = SCIL_BUFFER_ERR;
          goto decomp_cleanup;
      }
    }

    // ==================== Cleanup ============================================

    decomp_cleanup:
    free(unswaged_buffer);
    return ret;
}

// End repeat

scilI_algorithm_t algo_sigbits = {
    .c.DNtype = {
        CREATE_INITIALIZER(scil_sigbits)
    },
    "sigbits",
    3,
    SCIL_COMPRESSOR_TYPE_DATATYPES,
    1
};
