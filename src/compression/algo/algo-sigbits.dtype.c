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

#include <scil-swager.h>
#include <scil-util.h>

#include <math.h>
#include <string.h>

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
    9007199254740991,
    18014398509481983,
    36028797018963967,
    72057594037927935,
    144115188075855871,
    288230376151711743,
    576460752303423487,
    1152921504606846975,
    2305843009213693951,
    4611686018427387903
};

static int read_header(const byte* source,
                        size_t* source_size,
                        uint8_t* signs_id,
                        uint8_t* exponent_bit_count,
                        uint8_t* mantissa_bit_count,
                        int16_t* minimum_exponent,
                        double *fill_value,
                        uint64_t *fill_value_mask,
                        uint64_t *zero_value_mask){
    const byte * start = source;

    *signs_id = *((uint8_t*)source);
    source += 1;

    *exponent_bit_count = *((uint8_t*)source);
    source += 1;

    *mantissa_bit_count = *((uint8_t*)source);
    source += 1;

    *minimum_exponent = *((int16_t*)source);
    source += 2;

    scilU_unpack8(source, fill_value);
    source += 8;

    if(*fill_value != DBL_MAX){
      *fill_value_mask = *((uint64_t*)source);
      source += 8;
    }

    if(*minimum_exponent > 0){
      *zero_value_mask = *((uint64_t*)source);
      source += 8;
    }

    int size = (int) (source - start);
    *source_size -= size;
    return size;
}

static int write_header(byte* dest,
                         uint8_t signs_id,
                         uint8_t exponent_bit_count,
                         uint8_t mantissa_bit_count,
                         int16_t minimum_exponent,
                         double fill_value,
                         uint64_t fill_value_mask,
                         uint64_t zero_value_mask){
    byte * start = dest;

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
      *((uint64_t*)dest) = fill_value_mask;
      dest += 8;
    }

    if (minimum_exponent > 0){
      *((uint64_t*)dest) = zero_value_mask;
      dest += 8;
    }

    return (int) (dest - start);
}

static uint8_t calc_sign_bit_count(uint8_t minimum_sign, uint8_t maximum_sign){

    return minimum_sign == maximum_sign ? minimum_sign : 2;
}

static uint8_t calc_exponent_bit_count(int16_t minimum_exponent, int16_t maximum_exponent, uint16_t datatype_maximum, uint8_t flag_fill_value, uint8_t flag_zero_value){
    // Did the reservation for 1 rounding up exponent earlier
    const int16_t exp_delta = maximum_exponent - minimum_exponent;
    uint8_t bit_exponent_count = (uint8_t)ceil(log2(exp_delta + 1));

    if (flag_fill_value){
      if ((exp_delta == mask[bit_exponent_count]) && (bit_exponent_count + 1 < datatype_maximum))
        ++bit_exponent_count;
    }
    // Every value with exponent < finest_value_exponent will turn 0
    // but will be encoded with another special exponent to reduce
    // exponent_bit_count (real 0 always extents the range to the bottom)
    if (flag_zero_value){
      if ((exp_delta == mask[bit_exponent_count]) && (bit_exponent_count + 1 < datatype_maximum))
        ++bit_exponent_count;
    }

    return bit_exponent_count;
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
                                                  int16_t* maximum_exponent,
                                                  int16_t finest_exponent){

    *minimum_sign = 1;
    *maximum_sign = 0;

    *minimum_exponent = 0x7fff;
    *maximum_exponent = -*minimum_exponent;

    for(size_t i = 0; i < size; ++i){

        datatype_cast_<DATATYPE> cur;
        cur.f = buffer[i];

        if(cur.p.exponent >= finest_exponent) {
          if(*minimum_sign != 0 && cur.p.sign < *minimum_sign) { *minimum_sign = cur.p.sign; }
          if(*maximum_sign != 1 && cur.p.sign > *maximum_sign) { *maximum_sign = cur.p.sign; }

          if(cur.p.exponent < *minimum_exponent) { *minimum_exponent = cur.p.exponent; }
          if(cur.p.exponent > *maximum_exponent) { *maximum_exponent = cur.p.exponent; }
        }
    }

    // Maybe cur.p.exponent >= finest_exponent doesn't match for any value
    // then min/max still on useless init-value
    if (*maximum_exponent < *minimum_exponent) {
      *maximum_exponent = *minimum_exponent = finest_exponent;
    }
}

static void find_minimums_and_maximums_fill_<DATATYPE>(const <DATATYPE>* buffer,
                                                  const size_t size,
                                                  uint8_t* minimum_sign,
                                                  uint8_t* maximum_sign,
                                                  int16_t* minimum_exponent,
                                                  int16_t* maximum_exponent,
                                                  double fill_value,
                                                  int16_t finest_exponent,
                                                  byte *keys){

    *minimum_sign = 1;
    *maximum_sign = 0;

    *minimum_exponent = 0x7fff;
    *maximum_exponent = -*minimum_exponent;

    for(size_t i = 0; i < size; ++i) {
        if (buffer[i] != fill_value) {
            datatype_cast_<DATATYPE> cur;
            cur.f = buffer[i];

            if(cur.p.exponent >= finest_exponent) {
              if(*minimum_sign != 0 && cur.p.sign < *minimum_sign) { *minimum_sign = cur.p.sign; }
              if(*maximum_sign != 1 && cur.p.sign > *maximum_sign) { *maximum_sign = cur.p.sign; }

              if(cur.p.exponent < *minimum_exponent) { *minimum_exponent = cur.p.exponent; }
              if(cur.p.exponent > *maximum_exponent) { *maximum_exponent = cur.p.exponent; }

              /*It sets for each exponent existing in buffer 1 bit flag */
              keys[cur.p.exponent >> 3] |= 1 << (cur.p.exponent % 8);
            }
        }
    }
}

// TODO: Speed up shifts with lookup table.
static inline uint64_t compress_value_<DATATYPE>(<DATATYPE> value,
                                          uint8_t signs_id,
                                          uint8_t exponent_bit_count,
                                          uint8_t mantissa_bit_count,
                                          int16_t minimum_exponent,
                                          uint64_t zero_value_mask,
                                          uint64_t finest_value_mask){

    uint64_t result = 0;

    datatype_cast_<DATATYPE> cur;
    cur.f = value;

    // For minimum_exponent all values with exponent < finest value's exponent
    // are ignored, so if we see one here it is meant to be rounded
    // down to zero if another exponent smaller or up to finest
    if(cur.p.exponent < minimum_exponent - 1) {
        return zero_value_mask;
    } else if(cur.p.exponent < minimum_exponent) {
        return finest_value_mask;
    }

    // Calculate potentionally compressed sign bit, writing it and shifting to get space for exponent bits
    if(signs_id == 2){
        result = (uint64_t) cur.p.sign << exponent_bit_count;
    }

    // Amount of bitshifts to do at various points
    uint8_t shifts = MANTISSA_LENGTH_<DATATYPE_UPPER> - mantissa_bit_count;

    if(cur.p.exponent == MAX_EXPONENT_<DATATYPE>){ // with 1 sigbit we cannot encode infty
        // Infty is no problem as the mantissa is 0
        // NaN has any mantissa != 0, set the mantissa to 1 then
        result |= (uint64_t)(cur.p.exponent - minimum_exponent);
        result <<= mantissa_bit_count;
        result |= (cur.p.mantissa != 0);
        //printf("%lld %lld\n", result, cur.p.mantissa);
        return result;
    }

    // Calculating compressed mantissa with rounding
    uint64_t chkbit = (cur.p.mantissa >> (shifts - 1)) & 1;
    uint64_t mantissa_shifted = (cur.p.mantissa >> shifts) + chkbit;
    uint64_t sign_overflow = (1 << mantissa_bit_count) == mantissa_shifted;
    // Calculating compressed exponent with potential overflow from mantissa due to rounding up and writing it
    result |= (uint64_t)(cur.p.exponent - minimum_exponent + sign_overflow);

    // Shifting to get space for mantissa
    result <<= mantissa_bit_count;

    // Write significant bits of mantissa
    if(! sign_overflow){
      result |= mantissa_shifted;
    }
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
                                      int16_t minimum_exponent,
                                      uint64_t zero_value_mask){

    datatype_cast_<DATATYPE> finest;
    finest.p.sign = 0;
    finest.p.mantissa = 0;
    finest.p.exponent = minimum_exponent;
    uint64_t finest_value_mask = compress_value_<DATATYPE>(
                                          finest.f,
                                          signs_id,
                                          exponent_bit_count,
                                          mantissa_bit_count,
                                          minimum_exponent,
                                          zero_value_mask, 0);

    for(size_t i = 0; i < count; ++i){
        dest[i] = compress_value_<DATATYPE>(source[i], signs_id, exponent_bit_count, mantissa_bit_count, minimum_exponent, zero_value_mask, finest_value_mask);
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
                                      uint64_t fill_value_mask,
                                      uint64_t zero_value_mask){

    datatype_cast_<DATATYPE> finest;
    finest.p.sign = 0;
    finest.p.mantissa = 0;
    finest.p.exponent = minimum_exponent;
    uint64_t finest_value_mask = compress_value_<DATATYPE>(
                                          finest.f,
                                          signs_id,
                                          exponent_bit_count,
                                          mantissa_bit_count,
                                          minimum_exponent,
                                          zero_value_mask, 0);

    for(size_t i = 0; i < count; ++i){
      if (source[i] != fill_value){
        dest[i] = compress_value_<DATATYPE>(source[i], signs_id, exponent_bit_count, mantissa_bit_count, minimum_exponent, zero_value_mask, finest_value_mask);
      }else{
        dest[i] = fill_value_mask;
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
                                        int16_t minimum_exponent,
                                        uint64_t zero_value_mask){

    for (size_t i = 0; i < count; ++i) {
      if (source[i] == zero_value_mask){
        dest[i] = 0.0;
      } else {
        dest[i] = decompress_value_<DATATYPE>(source[i], bit_count_per_value, signs_id, exponent_bit_count, mantissa_bit_count, minimum_exponent);
      }
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
                                        uint64_t fill_value_mask,
                                        uint64_t zero_value_mask){
    for(size_t i = 0; i < count; ++i){
      if (source[i] == fill_value_mask){
        dest[i] = fill_value;
      } else if (source[i] == zero_value_mask){
        dest[i] = 0.0;
      } else {
        dest[i] = decompress_value_<DATATYPE>(source[i], bit_count_per_value, signs_id, exponent_bit_count, mantissa_bit_count, minimum_exponent);
      }
    }

    return SCIL_NO_ERR;
}

static void get_header_data_<DATATYPE>(const <DATATYPE>* source,
                                       size_t count,
                                       uint8_t* signs_id,
                                       uint8_t* exponent_bit_count,
                                       uint8_t mantissa_bit_count,
                                       int16_t* minimum_exponent,
                                       int16_t finest_exponent,
                                       uint64_t *zero_value_mask){

    uint8_t minimum_sign, maximum_sign;
    int16_t maximum_exponent;

    find_minimums_and_maximums_<DATATYPE>(source,
                                          count,
                                          &minimum_sign,
                                          &maximum_sign,
                                          minimum_exponent,
                                          &maximum_exponent,
                                          finest_exponent);

    int need_zero_value_mask = (*minimum_exponent > 0);

    // reserve 1 exponent for rounding up
    // will be used for zero_value_mask also
    if (maximum_exponent < MAX_EXPONENT_<DATATYPE>) {
        maximum_exponent++;
    }

    *signs_id = calc_sign_bit_count(minimum_sign, maximum_sign);
    *exponent_bit_count = calc_exponent_bit_count(*minimum_exponent, maximum_exponent, EXPONENT_LENGTH_<DATATYPE_UPPER> , 0, need_zero_value_mask);

    // A slim approach to get the zero_value_mask
    *zero_value_mask = 0;
    if (need_zero_value_mask) {
        datatype_cast_<DATATYPE> cur;
        cur.p.sign = 0;
        cur.p.mantissa = 0;
        cur.p.exponent = 0;
        if (maximum_exponent + 1 <= MAX_EXPONENT_<DATATYPE>) {
          cur.p.exponent = maximum_exponent + 1;
        }
        if (cur.p.exponent){
          *zero_value_mask = compress_value_<DATATYPE>(
                                              cur.f,
                                              *signs_id,
                                              *exponent_bit_count,
                                              mantissa_bit_count,
                                              *minimum_exponent,
                                              0, 0);
        }
    }
}

static void get_header_data_fill_<DATATYPE>(const <DATATYPE>* source,
                                       size_t count,
                                       uint8_t* signs_id,
                                       uint8_t* exponent_bit_count,
                                       uint8_t mantissa_bit_count,
                                       int16_t* minimum_exponent,
                                       double fill_value,
                                       uint64_t *fill_value_mask,
                                       int16_t finest_exponent,
                                       uint64_t *zero_value_mask){

    int16_t maximum_exponent;
    uint8_t minimum_sign, maximum_sign;

    byte *keys = (byte*)scilU_safe_malloc((EXPONENT_LENGTH_<DATATYPE_UPPER> - 1) << 2);
    memset(keys, 0, (EXPONENT_LENGTH_<DATATYPE_UPPER> - 1) << 2);

    find_minimums_and_maximums_fill_<DATATYPE>(source,
                                          count,
                                          &minimum_sign,
                                          &maximum_sign,
                                          minimum_exponent,
                                          &maximum_exponent,
                                          fill_value,
                                          finest_exponent,
                                          keys);

    /*printf("Check array with exponents from source\n");
    for(int i=0; i<32;i++)
      printf("%i ", keys[i]);*/

    // reserve 1 exponent for rounding up
    // will be used for fill_value_mask and zero_value_mask also
    if (maximum_exponent < MAX_EXPONENT_<DATATYPE>) {
        maximum_exponent++;
    }

    int need_zero_value_mask = (*minimum_exponent > 0);
    *signs_id = calc_sign_bit_count(minimum_sign, maximum_sign);
    *exponent_bit_count = calc_exponent_bit_count(*minimum_exponent, maximum_exponent, EXPONENT_LENGTH_<DATATYPE_UPPER> , 1, need_zero_value_mask);

    datatype_cast_<DATATYPE> cur;

    *fill_value_mask = 0;
    cur.p.sign = 0;
    cur.p.mantissa = 0;
    cur.p.exponent = 0;

    // TODO: Test this case. Does it happen? Extend to zero_value_mask
    if((*exponent_bit_count + 1 >= EXPONENT_LENGTH_<DATATYPE_UPPER>) && ((maximum_exponent - *minimum_exponent) != mask[*exponent_bit_count])){
      /* If we have maximal number of bits and
      we didn't increase the number of bits (because max-min diff != max)
      then...
      Find first free exponent number between min and max exponents
      (first zero bit number in keys will be set as cur.p.exponent) */
        int i = ((int)*minimum_exponent + 1) >> 3;
        int j = ((int)*minimum_exponent + 1) % 8;
        int imax = ((int)maximum_exponent >> 3) + (((int)maximum_exponent % 8)? 1:0);//32
        int jmax = 8;

        for(i; i < imax; i++){ //bytes
          if (i==imax-1) jmax = maximum_exponent % 8;
          if (keys[i] != 0xff){
            for(j; j < jmax; j++) //bits
              if (!(keys[i] >> j)){
                cur.p.exponent = (i << 3) + j;
              }
          }
          j=0;
        }
    } else {
      cur.p.exponent = maximum_exponent + 1;
    }

    /* If the free exponent was not found => error.
    We have no space to place fill value. */

    if (cur.p.exponent){
      *fill_value_mask = compress_value_<DATATYPE>(
                                          cur.f,
                                          *signs_id,
                                          *exponent_bit_count,
                                          mantissa_bit_count,
                                          *minimum_exponent,
                                          0, 0);
    }

    // A more slim approach to get the zero_value_mask
    *zero_value_mask = 0;
    if (need_zero_value_mask) {
        cur.p.exponent = 0;
        if (maximum_exponent + 2 <= MAX_EXPONENT_<DATATYPE>) {
          cur.p.exponent = maximum_exponent + 2;
        }
        if (cur.p.exponent){
          *zero_value_mask = compress_value_<DATATYPE>(
                                              cur.f,
                                              *signs_id,
                                              *exponent_bit_count,
                                              mantissa_bit_count,
                                              *minimum_exponent,
                                              0, 0);
        }
    }

    free(keys);
    return;
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

    // If neither hint 'sigbits' nor 'reltol' is given,
    // this initializes to -1 as unsigned = 255
    // and will fail the test mantissa_bit_count >= MANTISSA_LENGTH_<DATATYPE_UPPER>
    uint8_t mantissa_bit_count = ctx->hints.significant_bits - 1;

    // Calculate mantissa bits from hint 'reltol', apply when more strict
    if (ctx->hints.relative_tolerance_percent > 0.0) {
        uint8_t mantissa_bits_rel = scilU_relative_tolerance_to_significant_bits(ctx->hints.relative_tolerance_percent) - 1;
        if (ctx->hints.significant_bits == 0 || mantissa_bits_rel > mantissa_bit_count)
            mantissa_bit_count = mantissa_bits_rel;
    }
    //printf("#mantissa_bit_count = %d\n", mantissa_bit_count);

    /* Check for finest absolute tolerance.
       Intention is to reduce the amount of used exponents to save bits there.
       So we only need all exponents below finest_exponent to turn either
       to zero (must not be encoded by minimum possible exponent -> zero_value_mask)
       or to the finest value (use min. value with finest_exponent for continuity).
       Rounding threshold is half of "reduced finest", just 1 exponent less
    */
    <DATATYPE> finest_value = (<DATATYPE>) ctx->hints.relative_err_finest_abs_tolerance*2.0;
    datatype_cast_<DATATYPE> finest;
    finest.f = finest_value;

    // Check whether sigbit compression makes sense
    if(mantissa_bit_count == SCIL_ACCURACY_INT_FINEST || mantissa_bit_count >= MANTISSA_LENGTH_<DATATYPE_UPPER>){
        return SCIL_PRECISION_ERR;
    }

    size_t count = scil_dims_get_count(dims);

    uint8_t signs_id, exponent_bit_count;
    int16_t minimum_exponent;
    uint64_t fill_value_mask, zero_value_mask;

    if (ctx->hints.fill_value == DBL_MAX){
      get_header_data_<DATATYPE>(source, count, &signs_id, &exponent_bit_count, mantissa_bit_count, &minimum_exponent, finest.p.exponent, &zero_value_mask);
    }else{ // use the fill value
      get_header_data_fill_<DATATYPE>(source, count, &signs_id, &exponent_bit_count, mantissa_bit_count, &minimum_exponent, ctx->hints.fill_value, &fill_value_mask, finest.p.exponent, &zero_value_mask);

      if(!fill_value_mask){
        return SCIL_FILL_VAL_ERR;
      }
      //if(!zero_value_mask) is ok, it's just the normal 0.0 then
    }

    uint8_t bit_count_per_value = get_bit_count_per_value(signs_id, exponent_bit_count, mantissa_bit_count);

    // About finest: After finding min/max, the minimum_exponent will be the finest_exponent, so we use minimum_exponent from now on
    int header = write_header(dest, signs_id, exponent_bit_count, mantissa_bit_count, minimum_exponent, ctx->hints.fill_value, fill_value_mask, zero_value_mask);
    dest += header;

    *dest_size = round_up_byte(bit_count_per_value * count) + header;

    int ret = SCIL_NO_ERR;

    // ==================== Compression ========================================

    // Allocate intermediate buffer
    uint64_t* compressed_buffer = (uint64_t*)scilU_safe_malloc(count * sizeof(uint64_t));

    if (ctx->hints.fill_value == DBL_MAX){
      // Compress each value in source buffer
      if(compress_buffer_<DATATYPE>(compressed_buffer, source, count, signs_id, exponent_bit_count, mantissa_bit_count, minimum_exponent, zero_value_mask)){
        ret = SCIL_BUFFER_ERR;
        goto comp_cleanup;
      }
    }else{ // don't compress the fill value
      if(compress_buffer_fill_<DATATYPE>(compressed_buffer, source, count, signs_id, exponent_bit_count, mantissa_bit_count, minimum_exponent, ctx->hints.fill_value, fill_value_mask, zero_value_mask)){
        ret = SCIL_BUFFER_ERR;
        goto comp_cleanup;
      }
    }

    // Pack compressed values tightly
    if(scil_swage(dest, compressed_buffer, count, bit_count_per_value)){
        ret = SCIL_BUFFER_ERR;
        goto comp_cleanup;
    }
    /*printf("Control dest\n");
    for(size_t i = 0; i < *dest_size; ++i){
      printf("%i ", dest[i]);
    }*/

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

    double fill_value = DBL_MAX;

    // ==================== Initialization =====================================

    size_t count = scil_dims_get_count(dims);

    size_t source_size_cp = source_size;

    uint8_t signs_id, exponent_bit_count, mantissa_bit_count;
    int16_t minimum_exponent;
    uint64_t fill_value_mask = 0, zero_value_mask = 0;
    int header = read_header(source, &source_size_cp, &signs_id, &exponent_bit_count, &mantissa_bit_count, &minimum_exponent, &fill_value, &fill_value_mask, &zero_value_mask);
    source += header;

    uint8_t bit_count_per_value = get_bit_count_per_value(signs_id, exponent_bit_count, mantissa_bit_count);

    // ==================== Decompression ======================================

    uint64_t* unswaged_buffer = (uint64_t*)scilU_safe_malloc(count * sizeof(uint64_t));

    int ret = SCIL_NO_ERR;

    if(scil_unswage(unswaged_buffer, source, count, bit_count_per_value)){
        ret = SCIL_BUFFER_ERR;
        goto decomp_cleanup;
    }

    if (fill_value == DBL_MAX){
      // Deompress each value in source buffer
      if(decompress_buffer_<DATATYPE>(dest, unswaged_buffer, count, bit_count_per_value, signs_id, exponent_bit_count, mantissa_bit_count, minimum_exponent, zero_value_mask) ){
          ret = SCIL_BUFFER_ERR;
          goto decomp_cleanup;
      }
    }else{ // set fill value
      if(decompress_buffer_fill_<DATATYPE>(dest, unswaged_buffer, count, bit_count_per_value, signs_id, exponent_bit_count, mantissa_bit_count, minimum_exponent, fill_value, fill_value_mask, zero_value_mask) ){
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

scilU_algorithm_t algo_sigbits = {
    .c.DNtype = {
        CREATE_INITIALIZER(scil_sigbits)
    },
    "sigbits",
    3,
    SCIL_COMPRESSOR_TYPE_DATATYPES,
    1
};
