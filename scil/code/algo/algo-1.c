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
#include <algo/algo-1.h>

#include <math.h>

#include <scil-util.h>

static int find_min_max(DataType*restrict min, DataType*restrict max, const DataType* buf, const size_t size){

    assert(buf != NULL);

    *min = INFINITY;
    *max = -INFINITY;

    for(size_t i = 0; i < size; ++i)
    {
        if(buf[i] < *min)
            *min = buf[i];
        if(buf[i] > *max)
            *max = buf[i];
    }

    return 0;
}

static uint8_t get_needed_bit_count(const DataType min_value, const DataType max_value, const DataType absolute_tolerance){

    assert(max_value > min_value);
    assert(absolute_tolerance > 0);

    return (uint8_t)ceil(log2(1 + (max_value - min_value) / (2 * absolute_tolerance))); //TODO: Test this
}

static size_t round_up_byte(const size_t bits){

    uint8_t a = bits % 8;
    if(a == 0)
        return bits / 8;
    return 1 + (bits - a) / 8;
}

static uint64_t int_repres(const DataType num, const DataType min, const DataType absolute_tolerance){

    assert(num >= min);

    return (uint64_t)round((num - min) / (2 * absolute_tolerance));
}

static DataType DataType_repres(const uint64_t num, const DataType min, const DataType absolute_tolerance){

    return min + (DataType)num * 2 * absolute_tolerance;
}

static uint8_t get_bits(const uint64_t num, const uint8_t start, const uint8_t size){
    // TODO: Test this
    assert(start <= 64);
    assert(size <= 8);

    return (num << (64 - start)) >> (64 - size);
}

int scil_algo1_compress(const scil_context* ctx,
                        byte * restrict dest,
                        size_t* restrict dest_size,
                        const DataType*restrict source,
                        const size_t source_count)
{
    //Finding minimum and maximum values in data
    DataType min, max;
    find_min_max(&min, &max, source, source_count);

    //Locally assigning absolute tolerance
    DataType abs_tol = ctx->hints.absolute_tolerance;

    //Get needed bits per compressed number in data
    uint8_t bits_per_num = get_needed_bit_count(min, max, abs_tol);

    //Header byte size (currently sizes of magic number + min + abs_tol)
    uint8_t head_size = 1;

    //Set compression information
    //Minimum value
    *((DataType*)(dest)) = min;
    dest+= 8;
    head_size += 8;

    *((DataType*)(dest)) = abs_tol;
    dest+= 8;
    head_size += 8;

    *dest = bits_per_num;
    dest++;
    head_size += 1;

    //Get number of needed bytes for the whole compressed buffer
    *dest_size = round_up_byte(bits_per_num * source_count) + head_size;

    uint8_t end_mask[9] = {0, 255-127, 255-63, 255-31, 255-15, 255-7, 255-3, 255-1, 255};
    // 00000000 10000000 11000000 11100000 11110000 11111000 11111100 11111110 11111111

    size_t bit_index = 0;
    for(size_t i = 0; i < source_count; ++i){

        size_t start_byte = bit_index / 8;
        size_t end_byte = (bit_index + bits_per_num) / 8;

        uint64_t value = int_repres(source[i], min, abs_tol);

        uint8_t first_byte_fill = bit_index % 8;
        uint8_t first_byte_void = 8 - first_byte_fill;

        if(start_byte == end_byte){

            uint8_t bits = get_bits(value, bits_per_num, bits_per_num);

            dest[start_byte] &= end_mask[first_byte_fill];
            dest[start_byte] |= bits << (first_byte_void - bits_per_num);

        }else{

            // Start byte write
            uint8_t bits = get_bits(value, bits_per_num, first_byte_void);

            dest[start_byte] &= end_mask[first_byte_fill];
            dest[start_byte] |= bits;

            // Intermediate byte write
            for(size_t j = start_byte + 1; j < end_byte; ++j){

                uint8_t bit_start = bits_per_num - first_byte_void - (j - start_byte - 1) * 8;

                dest[j] = get_bits(value, bit_start, 8);
            }

            // End byte write
            uint8_t end_bit_size = bits_per_num - first_byte_void - (end_byte - start_byte - 1) * 8;
            bits = get_bits(value, end_bit_size, end_bit_size);

            dest[end_byte] = bits << (8 - end_bit_size);
        }

        bit_index += bits_per_num;
    }

    return 0;
}

int scil_algo1_decompress(  const scil_context* ctx,
                            DataType*restrict dest,
                            size_t*restrict dest_count,
                            const byte*restrict source,
                            const size_t source_size)
{
    assert(source != NULL);

    uint8_t bits_per_num;
    DataType min, abs_tol;

    size_t in_size = source_size - 17;

    // parse Header
    min = *((DataType*)(source));
    source+= 8;

    abs_tol = *((DataType*)(source));
    source+= 8;

    bits_per_num = *source;
    source++;

    size_t index = 0;
    for(size_t i = 0; i < in_size*8; i+=bits_per_num){

        // Get index of start and end byte
        size_t start_byte = i / 8;
        size_t end_byte = (i + bits_per_num) / 8;

        // # of bits in byte before bit index (i)
        uint8_t first_byte_before = i % 8;
        // # of bits in byte after bit index (i)
        uint8_t first_byte_after = 8 - first_byte_before;

        uint64_t value = 0;
        if(start_byte == end_byte){

            value = get_bits(source[start_byte], first_byte_after, bits_per_num);

        }else{

            // Get start byte relevant bits
            value = get_bits(source[start_byte], first_byte_after, first_byte_after);

            // Get intermediate bytes bits
            for(size_t j = start_byte + 1; j < end_byte; ++j){

                value <<= 8;
                value |= get_bits(source[j], 8, 8);
            }

            // Get last byte relevant bits
            uint8_t remaining_bits = bits_per_num - first_byte_after - (end_byte - start_byte - 1) * 8;

            if(remaining_bits != 0){

                value <<= remaining_bits;
                uint8_t bits = get_bits(source[end_byte], 8, remaining_bits);
                value |= bits;
                //printf("%lu\t%f\n", value, DataType_repres(value, min, abs_tol));
            }
        }

        dest[index] = DataType_repres(value, min, abs_tol);
        ++index;
    }

    *dest_count = index;

    return 0;
}

scil_compression_algorithm algo_algo1 = {
    scil_algo1_compress,
    scil_algo1_decompress,
    "algo1",
    1
};
