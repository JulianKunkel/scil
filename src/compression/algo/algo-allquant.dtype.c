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

#include <algo/algo-allquant.h>
#include <algo/huffman.h>

#include <scil.h>

#include <scil-error.h>
#include <scil-util.h>
#include <scil-quantizer.h>

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

static uint8_t calc_exponent_bit_count(int16_t minimum_exponent, int16_t maximum_exponent, uint16_t datatype_max_exponent){
    int16_t exp_delta = maximum_exponent - minimum_exponent;
    // reserve +1 exponent for rounding up
    if (maximum_exponent < datatype_max_exponent)
        exp_delta++;
    uint8_t exponent_bit_count = (uint8_t)ceil(log2(exp_delta + 1));
    return exponent_bit_count;
}

static uint8_t get_prefix_mask(uint8_t prefix_bit_count) {
    uint8_t mask = 1 << prefix_bit_count;
    mask--;
    mask <<= 8 - prefix_bit_count;
    return mask;
}

static uint64_t round_up_byte(const uint64_t bits){

    uint8_t a = bits % 8;
    if(a == 0)
        return bits / 8;
    return 1 + (bits - a) / 8;
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

// Taken from scil-swager.c, but now working with steps of 1 value

static uint8_t start_mask[9] = {
    255, //0b11111111
    127, //0b01111111
    63,  //0b00111111
    31,  //0b00011111
    15,  //0b00001111
    7,   //0b00000111
    3,   //0b00000011
    1,   //0b00000001
    0    //0b00000000
};

static uint8_t end_mask[9] = {
    0,         //0b00000000
    255 - 127, //0b10000000
    255 - 63,  //0b11000000
    255 - 31,  //0b11100000
    255 - 15,  //0b11110000
    255 - 7,   //0b11111000
    255 - 3,   //0b11111100
    255 - 1,   //0b11111110
    255        //0b11111111
};

// Can do negative right shifts!! (left shifts)
static byte right_shift_64(uint64_t value, int amount){
    return (byte)(amount >= 0 ?
        value >> amount :
        value << -amount);
}

static uint64_t right_shift_8(byte value, int amount){
    return (amount >= 0 ?
        ((uint64_t)value) >> amount :
        ((uint64_t)value) << -amount);
}

static void swage_value(byte* restrict buf_out,
                        const uint64_t value_in,
                        const uint8_t bit_count,
                        size_t* bit_index) {

    size_t start_byte = *bit_index / 8;
    size_t end_byte   = (*bit_index + bit_count) / 8;
    uint8_t bit_offset = *bit_index % 8;

    int8_t right_shifts = bit_count + bit_offset - 8;

    // Write to first byte

    // Set remaining bits in start_byte to 0
    buf_out[start_byte] &= end_mask[bit_offset];
    // Write as much bits of number as possible to remaining bits
    buf_out[start_byte] |= right_shift_64(value_in, right_shifts);

    // Write to following bytes
    uint8_t k = 1;
    for(uint64_t j = start_byte + 1; j <= end_byte; ++j)
    {
        buf_out[j] = right_shift_64(value_in, right_shifts - k * 8);
        ++k;
    }

    *bit_index += bit_count;
}

static void unswage_value(uint64_t* value_out,
                          const byte* restrict buf_in,
                          const uint8_t bit_count,
                          size_t* bit_index) {

    size_t start_byte = *bit_index / 8;                         // Index of starting byte of current value in swaged buffer
    size_t end_byte   = (*bit_index + bit_count) / 8;      // Index of ending byte of current value in swaged buffer

    uint8_t bit_offset = *bit_index % 8;                        // Index of current bit in byte [0-7]
    uint8_t end_byte_bits = (*bit_index + bit_count) % 8;  // Number of bits in end_byte occupied by current value

    int8_t right_shifts = 8 - bit_count - bit_offset;

    // Read from start_byte
    uint64_t intermed = right_shift_8(buf_in[start_byte] & start_mask[bit_offset], right_shifts); // Masks away first value-unrelated bits in start_byte and shifts related bits to final position
    // Read from intermediate bytes
    uint8_t k = 1;
    for(uint64_t j = start_byte + 1; j < end_byte; ++j)
    {
        intermed |= right_shift_8(buf_in[j], right_shifts + k * 8); // Shifts whole byte to final position and applies it
        ++k;
    }
    // Read from end_byte
    if(start_byte != end_byte)
    {
        intermed |= right_shift_8(buf_in[end_byte], 8 - end_byte_bits); // Shifts out unrelated end bits in end_byte and applies value
    }

    // Write to output buffer
    *value_out = intermed;

    *bit_index += bit_count;
}

//Supported datatypes: double float
// Repeat for each data type

// To optimize compression we need some statistics about the data.
// There are 5 relevant regions on the number line and maybe 1 special
// fill value:
//
// absneg | relneg | zero | relpos | abspos (fill)
//        ^        ^      ^        ^
// abstol-border  finest-border   abstol-border
//
// zero   Values below "finest" are allowed to be changed to 0.
//        Only values with an exponent < finest_exponent - 1 will do so.
//        Standard encoding of zero wastes a lot of bits, so they will get
//        a special encoding.
// rel*   For small values the relative error is relevant.
//        These Will be handled with reltol/sigbits algorithm.
// abs*   For big values the absolute error resulting from reltol-algo
//        gets too large. To be fine with "abstol" hint also, we need to
//        run abstol-algo (quantization) on those.
// fill   If fill value is set, this will be reproduced exactly as is.
//        Therefor stored in header data and encoded special.
//
// Values in different ranges are encoded differently, maybe with different
// amount of bits. We need to store wich algo was used per value. To do this
// most efficient we do some huffman code. Therefor we need statistics.
typedef struct region_stats_<DATATYPE> {
    datatype_cast_<DATATYPE> min;
    datatype_cast_<DATATYPE> max;
    size_t count;
    uint8_t prefix_mask;        // left aligned (better on decompression)
    uint8_t prefix_value;       // left aligned (better on decompression)
    uint8_t prefix_bit_count;
    uint8_t exponent_bit_count; // unused in quantization
    uint8_t mantissa_bit_count; // also used as quantize_bit_count
} region_stats_<DATATYPE>;

typedef struct allquant_stats_<DATATYPE> {
    region_stats_<DATATYPE> absneg;
    region_stats_<DATATYPE> relneg;
    region_stats_<DATATYPE> zero;
    region_stats_<DATATYPE> relpos;
    region_stats_<DATATYPE> abspos;
    region_stats_<DATATYPE> fill;
} allquant_stats_<DATATYPE>;

static void find_statistics_<DATATYPE>(const <DATATYPE>* buffer,
                                       const size_t size,
                                       allquant_stats_<DATATYPE>* stats,
                                       double fill_value,
                                       int16_t finest_exponent,
                                       int16_t abstol_min_exponent){

    stats->absneg.min.f = INFINITY_<DATATYPE>;
    stats->absneg.max.f = NINFINITY_<DATATYPE>;
    stats->absneg.count = 0;
    stats->relneg.min.f = INFINITY_<DATATYPE>;
    stats->relneg.max.f = NINFINITY_<DATATYPE>;
    stats->relneg.count = 0;
    stats->zero.min.f = INFINITY_<DATATYPE>;
    stats->zero.max.f = NINFINITY_<DATATYPE>;
    stats->zero.count = 0;
    stats->relpos.min.f = INFINITY_<DATATYPE>;
    stats->relpos.max.f = NINFINITY_<DATATYPE>;
    stats->relpos.count = 0;
    stats->abspos.min.f = INFINITY_<DATATYPE>;
    stats->abspos.max.f = NINFINITY_<DATATYPE>;
    stats->abspos.count = 0;
    stats->fill.min.f = (<DATATYPE>)fill_value;
    stats->fill.max.f = (<DATATYPE>)fill_value;
    stats->fill.count = 0;

    for(size_t i = 0; i < size; ++i){

        datatype_cast_<DATATYPE> cur;
        cur.f = buffer[i];

        if((double)cur.f == fill_value && fill_value != DBL_MAX) {
            stats->fill.count++;
        } else if(cur.p.exponent < finest_exponent - 1) {
            if(cur.f > stats->zero.max.f) stats->zero.max.f = cur.f;
            if(cur.f < stats->zero.min.f) stats->zero.min.f = cur.f;
            stats->zero.count++;
        } else if(cur.p.exponent < finest_exponent) {
            if(cur.p.sign) {
                stats->relneg.max.p.sign = 1;
                stats->relneg.max.p.exponent = finest_exponent;
                stats->relneg.max.p.mantissa = 0;
                if(stats->relneg.max.f < stats->relneg.min.f)
                    stats->relneg.min.f = stats->relneg.max.f;
                stats->relneg.count++;
            } else {
                stats->relpos.min.p.sign = 0;
                stats->relpos.min.p.exponent = finest_exponent;
                stats->relpos.min.p.mantissa = 0;
                if(stats->relpos.min.f > stats->relpos.max.f)
                    stats->relpos.max.f = stats->relpos.min.f;
                stats->relpos.count++;
            }
        } else if(cur.p.exponent < abstol_min_exponent) {
            if(cur.p.sign) {
                if(cur.f > stats->relneg.max.f) stats->relneg.max.f = cur.f;
                if(cur.f < stats->relneg.min.f) stats->relneg.min.f = cur.f;
                stats->relneg.count++;
            } else {
                if(cur.f > stats->relpos.max.f) stats->relpos.max.f = cur.f;
                if(cur.f < stats->relpos.min.f) stats->relpos.min.f = cur.f;
                stats->relpos.count++;
            }
        } else {
            if(cur.p.sign) {
                if(cur.f > stats->absneg.max.f) stats->absneg.max.f = cur.f;
                if(cur.f < stats->absneg.min.f) stats->absneg.min.f = cur.f;
                stats->absneg.count++;
            } else {
                if(cur.f > stats->abspos.max.f) stats->abspos.max.f = cur.f;
                if(cur.f < stats->abspos.min.f) stats->abspos.min.f = cur.f;
                stats->abspos.count++;
            }
        }
    }
}

static uint64_t get_bit_count_region_<DATATYPE>(region_stats_<DATATYPE>* region) {
    // printf("pre %d exp %d mant %d count %d\n", region->prefix_bit_count, region->exponent_bit_count, region->mantissa_bit_count, region->count);
    return (region->prefix_bit_count + region->exponent_bit_count +
        region->mantissa_bit_count) * region->count;
}

static uint64_t get_bit_count_all_<DATATYPE>(allquant_stats_<DATATYPE>* stats) {
    return get_bit_count_region_<DATATYPE>(&stats->absneg) +
        get_bit_count_region_<DATATYPE>(&stats->relneg) +
        get_bit_count_region_<DATATYPE>(&stats->zero) +
        get_bit_count_region_<DATATYPE>(&stats->relpos) +
        get_bit_count_region_<DATATYPE>(&stats->abspos) +
        get_bit_count_region_<DATATYPE>(&stats->fill);
}

static void get_header_data_<DATATYPE>(const <DATATYPE>* source,
                                       size_t count,
                                       allquant_stats_<DATATYPE>* stats,
                                       double fill_value,
                                       int16_t finest_exponent,
                                       double abstol,
                                       int16_t abstol_min_exponent,
                                       uint8_t mantissa_bit_count){

    find_statistics_<DATATYPE>(source,
                               count,
                               stats,
                               fill_value,
                               finest_exponent,
                               abstol_min_exponent);

    // Huffman encode prefix bits for regions
    huffman_entity huffman[6];
    huffman[0].count = stats->absneg.count;
    huffman[1].count = stats->relneg.count;
    huffman[2].count = stats->zero.count;
    huffman[3].count = stats->relpos.count;
    huffman[4].count = stats->abspos.count;
    huffman[5].count = stats->fill.count;
    huffman_encode(huffman, 6);

    // Store different bit counts per region
    stats->absneg.prefix_mask = huffman[0].bitmask;
    stats->absneg.prefix_value = huffman[0].bitvalue;
    stats->absneg.prefix_bit_count = huffman[0].bitcount;
    stats->absneg.exponent_bit_count = 0;
    stats->absneg.mantissa_bit_count = scil_calculate_bits_needed_<DATATYPE>(
      stats->absneg.min.f, stats->absneg.max.f, abstol, 0, NULL);

    stats->relneg.prefix_mask = huffman[1].bitmask;
    stats->relneg.prefix_value = huffman[1].bitvalue;
    stats->relneg.prefix_bit_count = huffman[1].bitcount;
    stats->relneg.exponent_bit_count = calc_exponent_bit_count(
      stats->relneg.max.p.exponent, stats->relneg.min.p.exponent,
      MAX_EXPONENT_<DATATYPE>);
    stats->relneg.mantissa_bit_count = mantissa_bit_count;

    stats->zero.prefix_mask = huffman[2].bitmask;
    stats->zero.prefix_value = huffman[2].bitvalue;
    stats->zero.prefix_bit_count = huffman[2].bitcount;
    stats->zero.exponent_bit_count = 0;
    stats->zero.mantissa_bit_count = 0;

    stats->relpos.prefix_mask = huffman[3].bitmask;
    stats->relpos.prefix_value = huffman[3].bitvalue;
    stats->relpos.prefix_bit_count = huffman[3].bitcount;
    stats->relpos.exponent_bit_count = calc_exponent_bit_count(
      stats->relpos.min.p.exponent, stats->relpos.max.p.exponent,
      MAX_EXPONENT_<DATATYPE>);
    stats->relpos.mantissa_bit_count = mantissa_bit_count;

    stats->abspos.prefix_mask = huffman[4].bitmask;
    stats->abspos.prefix_value = huffman[4].bitvalue;
    stats->abspos.prefix_bit_count = huffman[4].bitcount;
    stats->abspos.exponent_bit_count = 0;
    stats->abspos.mantissa_bit_count = scil_calculate_bits_needed_<DATATYPE>(
      stats->abspos.min.f, stats->abspos.max.f, abstol, 0, NULL);

    stats->fill.prefix_mask = huffman[5].bitmask;
    stats->fill.prefix_value = huffman[5].bitvalue;
    stats->fill.prefix_bit_count = huffman[5].bitcount;
    stats->fill.exponent_bit_count = 0;
    stats->fill.mantissa_bit_count = 0;
}

static int read_header_<DATATYPE>(const byte* source,
                        size_t* source_size,
                        allquant_stats_<DATATYPE>* stats,
                        double *abstol,
                        double *fill_value){
    const byte* start = source;

    uint8_t region_flags = *((uint8_t*)source);
    ++source;

    // mantissa_bit_count is always equal in relneg and relpos
    if (region_flags & 10) {
        stats->relneg.mantissa_bit_count = *((uint8_t*)source);
        stats->relpos.mantissa_bit_count = *((uint8_t*)source);
        ++source;
    }

    // need abstol value if used in at least one region
    if (region_flags & 17) {
        scilU_unpack8(source, abstol);
        source += 8;
    }

    if (region_flags & 1) { // absneg
        stats->absneg.prefix_value = *((uint8_t*)source);
        ++source;
        stats->absneg.prefix_bit_count = *((uint8_t*)source);
        ++source;
        stats->absneg.mantissa_bit_count = *((uint8_t*)source);
        ++source;
        double d;
        scilU_unpack8(source, (&d));
        stats->absneg.min.f = (<DATATYPE>)d;
        source += 8;
        stats->absneg.prefix_mask = get_prefix_mask(stats->absneg.prefix_bit_count);
    } else {
        stats->absneg.prefix_mask = 0;
        stats->absneg.prefix_value = 1;
        stats->absneg.prefix_bit_count = 0;
        stats->absneg.mantissa_bit_count = 0;
    }
    stats->absneg.exponent_bit_count = 0;

    if (region_flags & 2) { // relneg
        stats->relneg.prefix_value = *((uint8_t*)source);
        ++source;
        stats->relneg.prefix_bit_count = *((uint8_t*)source);
        ++source;
        stats->relneg.exponent_bit_count = *((uint8_t*)source);
        ++source;
        stats->relneg.max.f = 1.0;
        stats->relneg.max.p.exponent = *((int16_t*)source);
        source += 2;
        stats->relneg.prefix_mask = get_prefix_mask(stats->relneg.prefix_bit_count);
    } else {
        stats->relneg.prefix_mask = 0;
        stats->relneg.prefix_value = 1;
        stats->relneg.prefix_bit_count = 0;
        stats->relneg.exponent_bit_count = 0;
        stats->relneg.mantissa_bit_count = 0;
    }

    if (region_flags & 4) { // zero
        stats->zero.prefix_value = *((uint8_t*)source);
        ++source;
        stats->zero.prefix_bit_count = *((uint8_t*)source);
        ++source;
        stats->zero.prefix_mask = get_prefix_mask(stats->zero.prefix_bit_count);
    } else {
        stats->zero.prefix_mask = 0;
        stats->zero.prefix_value = 1;
        stats->zero.prefix_bit_count = 0;
    }
    stats->zero.exponent_bit_count = 0;
    stats->zero.mantissa_bit_count = 0;

    if (region_flags & 8) { // relpos
        stats->relpos.prefix_value = *((uint8_t*)source);
        ++source;
        stats->relpos.prefix_bit_count = *((uint8_t*)source);
        ++source;
        stats->relpos.exponent_bit_count = *((uint8_t*)source);
        ++source;
        stats->relpos.min.f = 1.0;
        stats->relpos.min.p.exponent = *((int16_t*)source);
        source += 2;
        stats->relpos.prefix_mask = get_prefix_mask(stats->relpos.prefix_bit_count);
    } else {
        stats->relpos.prefix_mask = 0;
        stats->relpos.prefix_value = 1;
        stats->relpos.prefix_bit_count = 0;
        stats->relpos.exponent_bit_count = 0;
        stats->relpos.mantissa_bit_count = 0;
    }

    if (region_flags & 16) { // abspos
        stats->abspos.prefix_value = *((uint8_t*)source);
        ++source;
        stats->abspos.prefix_bit_count = *((uint8_t*)source);
        ++source;
        stats->abspos.mantissa_bit_count = *((uint8_t*)source);
        ++source;
        double d;
        scilU_unpack8(source, (&d));
        stats->abspos.min.f = (<DATATYPE>)d;
        source += 8;
        stats->abspos.prefix_mask = get_prefix_mask(stats->abspos.prefix_bit_count);
    } else {
        stats->abspos.prefix_mask = 0;
        stats->abspos.prefix_value = 1;
        stats->abspos.prefix_bit_count = 0;
        stats->abspos.mantissa_bit_count = 0;
    }
    stats->abspos.exponent_bit_count = 0;

    if (region_flags & 32) { // fill
        stats->fill.prefix_value = *((uint8_t*)source);
        ++source;
        stats->fill.prefix_bit_count = *((uint8_t*)source);
        ++source;
        scilU_unpack8(source, fill_value);
        source += 8;
        stats->fill.prefix_mask = get_prefix_mask(stats->fill.prefix_bit_count);
    } else {
        stats->fill.prefix_mask = 0;
        stats->fill.prefix_value = 1;
        stats->fill.prefix_bit_count = 0;
    }
    stats->fill.exponent_bit_count = 0;
    stats->fill.mantissa_bit_count = 0;

    int size = (int) (source - start);
    *source_size -= size;
    return size;
}

static int write_header_<DATATYPE>(byte* dest,
                                   allquant_stats_<DATATYPE>* stats,
                                   double abstol,
                                   double fill_value){
    byte* start = dest;

    uint8_t region_flags = 0;
    if (stats->absneg.count > 0)  region_flags |= 1;
    if (stats->relneg.count > 0)  region_flags |= 2;
    if (stats->zero.count > 0)    region_flags |= 4;
    if (stats->relpos.count > 0)  region_flags |= 8;
    if (stats->abspos.count > 0)  region_flags |= 16;
    if (stats->fill.count > 0)    region_flags |= 32;

    *dest = region_flags;
    ++dest;

    // mantissa_bit_count is always equal in relneg and relpos
    if (region_flags & 10) {
        *dest = stats->relneg.mantissa_bit_count;
        ++dest;
    }

    // need to store abstol if used in at least one region
    if (region_flags & 17) {
        scilU_pack8(dest, abstol);
        dest += 8;
    }

    if (region_flags & 1) { // absneg
        *dest = stats->absneg.prefix_value;
        ++dest;
        *dest = stats->absneg.prefix_bit_count;
        ++dest;
        *dest = stats->absneg.mantissa_bit_count;
        ++dest;
        double d = stats->absneg.min.f;
        scilU_pack8(dest, d);
        dest += 8;
    }

    if (region_flags & 2) { // relneg
        *dest = stats->relneg.prefix_value;
        ++dest;
        *dest = stats->relneg.prefix_bit_count;
        ++dest;
        *dest = stats->relneg.exponent_bit_count;
        ++dest;
        *((int16_t*)dest) = stats->relneg.max.p.exponent;
        dest += 2;
    }

    if (region_flags & 4) { // zero
        *dest = stats->zero.prefix_value;
        ++dest;
        *dest = stats->zero.prefix_bit_count;
        ++dest;
    }

    if (region_flags & 8) { // relpos
        *dest = stats->relpos.prefix_value;
        ++dest;
        *dest = stats->relpos.prefix_bit_count;
        ++dest;
        *dest = stats->relpos.exponent_bit_count;
        ++dest;
        *((int16_t*)dest) = stats->relpos.min.p.exponent;
        dest += 2;
    }

    if (region_flags & 16) { // abspos
        *dest = stats->abspos.prefix_value;
        ++dest;
        *dest = stats->abspos.prefix_bit_count;
        ++dest;
        *dest = stats->abspos.mantissa_bit_count;
        ++dest;
        double d = stats->abspos.min.f;
        scilU_pack8(dest, d);
        dest += 8;
    }

    if (region_flags & 32) { // fill
        *dest = stats->fill.prefix_value;
        ++dest;
        *dest = stats->fill.prefix_bit_count;
        ++dest;
        scilU_pack8(dest, fill_value);
        dest += 8;
    }

    return (int) (dest - start);
}

// TODO: Speed up shifts with lookup table.
static inline uint64_t compress_value_<DATATYPE>(<DATATYPE> value,
                                          uint8_t exponent_bit_count,
                                          uint8_t mantissa_bit_count,
                                          int16_t minimum_exponent){

    uint64_t result = 0;

    datatype_cast_<DATATYPE> cur;
    cur.f = value;

    // Checking for finest and sign-bit was done earlier

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
                                              uint8_t exponent_bit_count,
                                              uint8_t mantissa_bit_count,
                                              int16_t minimum_exponent){

    datatype_cast_<DATATYPE> cur;

    cur.p.sign     = 0; // sign is stored in prefix and handled outside
    cur.p.exponent = get_exponent(value, exponent_bit_count, mantissa_bit_count, minimum_exponent);
    cur.p.mantissa = get_mantissa_<DATATYPE>(value, mantissa_bit_count);

    return cur.f;
}

static inline uint64_t quantize_value_<DATATYPE>(<DATATYPE> value,
                                                 double absolute_tolerance,
                                                 <DATATYPE> minimum){

    return (((uint64_t) ( ((double) (value - minimum)) / absolute_tolerance )) + 1)>>1;
}

static inline <DATATYPE> unquantize_value_<DATATYPE>(uint64_t value,
                                                    double absolute_tolerance,
                                                    <DATATYPE> minimum){

    return minimum + (<DATATYPE>)(value * 2 * absolute_tolerance);
}

static int compress_buffer_<DATATYPE>(byte * restrict dest,
                                      const <DATATYPE>* restrict source,
                                      size_t count,
                                      allquant_stats_<DATATYPE>* stats,
                                      double fill_value,
                                      int16_t finest_exponent,
                                      double abstol,
                                      int16_t abstol_min_exponent){

    // Swaging state
    size_t bit_index = 0;
    uint64_t unswaged;

    // Precalculate 64bit representation of finest value
    datatype_cast_<DATATYPE> finest;
    finest.p.sign = 0;
    finest.p.mantissa = 0;
    finest.p.exponent = finest_exponent;
    uint64_t finest_neg = compress_value_<DATATYPE>(finest.f,
        stats->relneg.exponent_bit_count, stats->relneg.mantissa_bit_count,
        stats->relneg.max.p.exponent);
    uint64_t finest_pos = compress_value_<DATATYPE>(finest.f,
        stats->relpos.exponent_bit_count, stats->relpos.mantissa_bit_count,
        stats->relpos.min.p.exponent);

    // Precalculate amount of data bits for rel-regions
    uint8_t relneg_data_bit_count = stats->relneg.exponent_bit_count +
        stats->relneg.mantissa_bit_count;
    uint8_t relpos_data_bit_count = stats->relpos.exponent_bit_count +
        stats->relpos.mantissa_bit_count;

    // Convert prefix values from left aligned to right aligned
    // because swaging expects given number of bits right aligned within 64bit
    uint64_t absneg_prefix_value = stats->absneg.prefix_value >> (8 -
        stats->absneg.prefix_bit_count);
    uint64_t relneg_prefix_value = stats->relneg.prefix_value >> (8 -
        stats->relneg.prefix_bit_count);
    uint64_t zero_prefix_value = stats->zero.prefix_value >> (8 -
        stats->zero.prefix_bit_count);
    uint64_t relpos_prefix_value = stats->relpos.prefix_value >> (8 -
        stats->relpos.prefix_bit_count);
    uint64_t abspos_prefix_value = stats->abspos.prefix_value >> (8 -
        stats->abspos.prefix_bit_count);
    uint64_t fill_prefix_value = stats->fill.prefix_value >> (8 -
        stats->fill.prefix_bit_count);

    // For each value:
    // - check wich region value belongs to
    // - write regions huffman prefix (variable length)
    // stop on special cases, else
    // - compress value to unswaged 64bit with sigbits- or abstol-algo
    // - swage that value according to regions bit_counts

    for(size_t i = 0; i < count; ++i) {
        datatype_cast_<DATATYPE> cur;
        cur.f = source[i];

        if((double)cur.f == fill_value && fill_value != DBL_MAX) {
            swage_value(dest, fill_prefix_value,
                stats->fill.prefix_bit_count, &bit_index);
        } else if(cur.p.exponent < finest_exponent - 1) {
            swage_value(dest, zero_prefix_value,
                stats->zero.prefix_bit_count, &bit_index);
        } else if(cur.p.exponent < finest_exponent) {
            if(cur.p.sign) {
                swage_value(dest, relneg_prefix_value,
                    stats->relneg.prefix_bit_count, &bit_index);
                swage_value(dest, finest_neg,
                    relneg_data_bit_count, &bit_index);
            } else {
                swage_value(dest, relpos_prefix_value,
                    stats->relpos.prefix_bit_count, &bit_index);
                swage_value(dest, finest_pos,
                    relpos_data_bit_count, &bit_index);
            }
        } else if(cur.p.exponent < abstol_min_exponent) {
            if(cur.p.sign) {
                swage_value(dest, relneg_prefix_value,
                    stats->relneg.prefix_bit_count, &bit_index);
                // Compress_value needs min_exponent, but caution:
                // For negative values this is the exponent of the max
                // value in this range!
                unswaged = compress_value_<DATATYPE>(source[i],
                    stats->relneg.exponent_bit_count,
                    stats->relneg.mantissa_bit_count,
                    stats->relneg.max.p.exponent);
                swage_value(dest, unswaged,
                    relneg_data_bit_count, &bit_index);
            } else {
                swage_value(dest, relpos_prefix_value,
                    stats->relpos.prefix_bit_count, &bit_index);
                unswaged = compress_value_<DATATYPE>(source[i],
                    stats->relpos.exponent_bit_count,
                    stats->relpos.mantissa_bit_count,
                    stats->relpos.min.p.exponent);
                swage_value(dest, unswaged,
                    relpos_data_bit_count, &bit_index);
            }
        } else {
            if(cur.p.sign) {
                unswaged = quantize_value_<DATATYPE>(source[i], abstol,
                    stats->absneg.min.f);
                swage_value(dest, absneg_prefix_value,
                    stats->absneg.prefix_bit_count, &bit_index);
                swage_value(dest, unswaged,
                    stats->absneg.mantissa_bit_count, &bit_index);
            } else {
                unswaged = quantize_value_<DATATYPE>(source[i], abstol,
                    stats->abspos.min.f);
                swage_value(dest, abspos_prefix_value,
                    stats->abspos.prefix_bit_count, &bit_index);
                swage_value(dest, unswaged,
                    stats->abspos.mantissa_bit_count, &bit_index);
            }
        }
    }

    return SCIL_NO_ERR;
}

static int decompress_buffer_<DATATYPE>(<DATATYPE>* restrict dest,
                                        const byte* restrict source,
                                        size_t count,
                                        allquant_stats_<DATATYPE>* stats,
                                        double abstol,
                                        double fill_value){

    // Swaging state
    size_t bit_index = 0;
    uint64_t unswaged;
    uint8_t prefix_byte;

    // Precalculate amount of data bits for rel-regions
    uint8_t relneg_data_bit_count = stats->relneg.exponent_bit_count +
        stats->relneg.mantissa_bit_count;
    uint8_t relpos_data_bit_count = stats->relpos.exponent_bit_count +
        stats->relpos.mantissa_bit_count;

    for (size_t i = 0; i < count; ++i) {
      // Read 1 byte, then mask it with regions bitmask and compare with
      // regions value. By design only one will match and unused regions
      // are set up to never match.
      // After the region is identified, we know how many prefix bits it was
      // and then rewind some bits, because we did not read a full byte.
      // From knowing the region we then know how many data bits to read next.

      unswage_value(&unswaged, source, 8, &bit_index);
      prefix_byte = (uint8_t)unswaged;

      if ((prefix_byte & stats->zero.prefix_mask) ==
        stats->zero.prefix_value) {
          bit_index -= 8 - stats->zero.prefix_bit_count;
          dest[i] = 0.0;
      } else if ((prefix_byte & stats->fill.prefix_mask) ==
        stats->fill.prefix_value) {
          bit_index -= 8 - stats->fill.prefix_bit_count;
          dest[i] = (<DATATYPE>)fill_value;
      } else if ((prefix_byte & stats->relneg.prefix_mask) ==
        stats->relneg.prefix_value) {
          bit_index -= 8 - stats->relneg.prefix_bit_count;
          unswage_value(&unswaged, source, relneg_data_bit_count, &bit_index);
          dest[i] = -decompress_value_<DATATYPE>(unswaged,
            stats->relneg.exponent_bit_count, stats->relneg.mantissa_bit_count,
            stats->relneg.max.p.exponent);
      } else if ((prefix_byte & stats->relpos.prefix_mask) ==
        stats->relpos.prefix_value) {
          bit_index -= 8 - stats->relpos.prefix_bit_count;
          unswage_value(&unswaged, source, relpos_data_bit_count, &bit_index);
          dest[i] = decompress_value_<DATATYPE>(unswaged,
            stats->relpos.exponent_bit_count, stats->relpos.mantissa_bit_count,
            stats->relpos.min.p.exponent);
      } else if ((prefix_byte & stats->absneg.prefix_mask) ==
        stats->absneg.prefix_value) {
          bit_index -= 8 - stats->absneg.prefix_bit_count;
          unswage_value(&unswaged, source, stats->absneg.mantissa_bit_count, &bit_index);
          dest[i] = unquantize_value_<DATATYPE>(unswaged, abstol,
              stats->absneg.min.f);
      } else if ((prefix_byte & stats->abspos.prefix_mask) ==
        stats->abspos.prefix_value) {
          bit_index -= 8 - stats->abspos.prefix_bit_count;
          unswage_value(&unswaged, source, stats->abspos.mantissa_bit_count, &bit_index);
          dest[i] = unquantize_value_<DATATYPE>(unswaged, abstol,
              stats->abspos.min.f);
      } else {
          // Corrupted data, found illegal prefix.
          // Due to huffman codes, this would mean the prefixes from header
          // were corrupt. Should never happen.
          return SCIL_BUFFER_ERR;
      }
    }
    return SCIL_NO_ERR;
}

int scil_allquant_compress_<DATATYPE>(const scil_context_t* ctx,
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
    double reltol = ctx->hints.relative_tolerance_percent;
    if (reltol > 0.0) {
        uint8_t mantissa_bits_rel = scilU_relative_tolerance_to_significant_bits(reltol) - 1;
        if (ctx->hints.significant_bits == 0 || mantissa_bits_rel > mantissa_bit_count)
            mantissa_bit_count = mantissa_bits_rel;
    } else {
        // store sigbits given as reltol for later abstol check
        reltol = scilU_significant_bits_to_relative_tolerance(ctx->hints.significant_bits);
    }
    //printf("#mantissa_bit_count = %d\n", mantissa_bit_count);

    /* Check for finest absolute tolerance.
       Intention is to reduce the amount of used exponents to save bits there.
       So we only need all exponents below finest_exponent to turn either
       to zero (must not be encoded by minimum possible exponent)
       or to the finest value (use min. value with finest_exponent for continuity).
       Rounding threshold is half of "reduced finest", just 1 exponent less
    */
    <DATATYPE> finest_value = (<DATATYPE>) ctx->hints.relative_err_finest_abs_tolerance*2.0;
    datatype_cast_<DATATYPE> finest;
    finest.f = finest_value;
    int16_t finest_exponent = finest.p.exponent;

    // Check whether sigbit compression makes sense (base to allquant)
    if(mantissa_bit_count == SCIL_ACCURACY_INT_FINEST || mantissa_bit_count >= MANTISSA_LENGTH_<DATATYPE_UPPER>){
        return SCIL_PRECISION_ERR;
    }

    /* Check for absolute tolerance
       To match reltol AND abstol, there is a value x where the errors equal
       x + abstol = x * (1 + reltol/100) ==> x = 100 * abstol / reltol
       for any value <= x abstol is fine using reltol (sigbits) algo.
       for any value > x absolute error of reltol algo gets too big,
       need to implement abstol/quantize algo. for that range
    */
    double abstol = ctx->hints.absolute_tolerance;
    int16_t abstol_min_exponent = MAX_EXPONENT_<DATATYPE> + 1; // no abstol
    if (abstol > 0.0) {
        <DATATYPE> abstol_min_value = (<DATATYPE>)((abstol / reltol) * 100.0);
        datatype_cast_<DATATYPE> abstol_min;
        abstol_min.f = abstol_min_value;
        abstol_min_exponent = abstol_min.p.exponent + 1; //TODO explain + 1

        // Turning finest into 0 will obviously cause an abs-error of finest
        //if(finest_value > abstol){
        //    return SCIL_PRECISION_ERR;
        //}
    }

    size_t count = scil_dims_get_count(dims);

    allquant_stats_<DATATYPE> stats; // stores all statistics and bitcounts

    get_header_data_<DATATYPE>(source, count, &stats, ctx->hints.fill_value,
      finest_exponent, abstol, abstol_min_exponent, mantissa_bit_count);

    uint64_t bit_count_all = get_bit_count_all_<DATATYPE>(&stats);

    int header = write_header_<DATATYPE>(dest, &stats, abstol, ctx->hints.fill_value);

    *dest_size = round_up_byte(bit_count_all) + header;

    // ==================== Compression ========================================

    // Compress and pack / swage per value, as bits_per_value depends on
    // the region the value is in
    if(compress_buffer_<DATATYPE>(dest + header, source, count, &stats,
      ctx->hints.fill_value, finest_exponent, abstol, abstol_min_exponent)) {
        return SCIL_BUFFER_ERR;
    }
    return SCIL_NO_ERR;
}

int scil_allquant_decompress_<DATATYPE>(<DATATYPE>*restrict dest,
                                       scil_dims_t* dims,
                                       byte*restrict source,
                                       size_t source_size){

    assert(dest != NULL);
    assert(dims != NULL);
    assert(source != NULL);

    double fill_value = DBL_MAX;
    double abstol = 0.0;

    // ==================== Initialization =====================================

    size_t count = scil_dims_get_count(dims);

    size_t source_size_cp = source_size;

    allquant_stats_<DATATYPE> stats;

    int header = read_header_<DATATYPE>(source, &source_size_cp, &stats, &abstol, &fill_value);

    // ==================== Decompression ======================================

    // Deompress each value in source buffer
    if(decompress_buffer_<DATATYPE>(dest, source + header, count, &stats, abstol, fill_value)) {
        return SCIL_BUFFER_ERR;
    }
    return SCIL_NO_ERR;
}

// End repeat

scilU_algorithm_t algo_allquant = {
    .c.DNtype = {
        CREATE_INITIALIZER(scil_allquant)
    },
    "allquant",
    12,
    SCIL_COMPRESSOR_TYPE_DATATYPES,
    1
};
