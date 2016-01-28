#include <math.h>

#include <algo/algo-1.h>
#include <util.h>

static int find_min_max(double*restrict min, double*restrict max, const double* buf, const size_t size){

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

static uint8_t get_needed_bit_count(const double min_value, const double max_value, const double absolute_tolerance){

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

static uint64_t int_repres(const double num, const double min, const double absolute_tolerance){

    assert(num >= min);

    return (uint64_t)round((num - min) / (2 * absolute_tolerance));
}

static double double_repres(const uint64_t num, const double min, const double absolute_tolerance){

    return min + (double)num * 2 * absolute_tolerance;
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
                        const double*restrict source,
                        const size_t source_count)
{
    //Finding minimum and maximum values in data
    double min, max;
    find_min_max(&min, &max, source, source_count);

    //Locally assigning absolute tolerance
    double abs_tol = ctx->hints.absolute_tolerance;

    //Get needed bits per compressed number in data
    uint8_t bits_per_num = get_needed_bit_count(min, max, abs_tol);

    //Header byte size (currently sizes of magic number + min + abs_tol)
    uint8_t head_size = 1;

    //Set compression information
    //Minimum value
    *((double*)(dest)) = min;
    dest+= 8;
    head_size += 8;

    *((double*)(dest)) = abs_tol;
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
                            double*restrict dest,
                            size_t*restrict dest_count,
                            const byte*restrict source,
                            const size_t source_size)
{
    assert(source != NULL);

    uint8_t bits_per_num;
    double min, abs_tol;

    size_t in_size = source_size - 17;

    // parse Header
    min = *((double*)(source));
    source+= 8;

    abs_tol = *((double*)(source));
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
                //printf("%lu\t%f\n", value, double_repres(value, min, abs_tol));
            }
        }

        dest[index] = double_repres(value, min, abs_tol);
        ++index;
    }

    *dest_count = index;

    return 0;
}
