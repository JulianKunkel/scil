#include <scil-swager.h>


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

int scil_swage(byte* restrict buf_out,
               const uint64_t* restrict buf_in,
               const size_t count,
               const uint8_t bits_per_value)
{
    size_t bit_index = 0;
    for(size_t i = 0; i < count; ++i)
    {
        size_t start_byte = bit_index / 8;
        size_t end_byte   = (bit_index + bits_per_value) / 8;
        uint8_t bit_offset = bit_index % 8;

        int8_t right_shifts = bits_per_value + bit_offset - 8;

        // Write to first byte

        // Set remaining bits in start_byte to 0
        buf_out[start_byte] &= end_mask[bit_offset];
        // Write as much bits of number as possible to remaining bits
        buf_out[start_byte] |= right_shift_64(buf_in[i], right_shifts);

        // Write to following bytes
        uint8_t k = 1;
        for(uint64_t j = start_byte + 1; j <= end_byte; ++j)
        {
            buf_out[j] = right_shift_64(buf_in[i], right_shifts - k * 8);
            ++k;
        }

        bit_index += bits_per_value;
    }

    return 0;
}

int scil_unswage(uint64_t* restrict buf_out,
                 const byte* restrict buf_in,
                 const size_t count,
                 const uint8_t bits_per_value)
{
    size_t bit_index = 0;
    for(size_t i = 0; i < count; ++i)
    {
        size_t start_byte = bit_index / 8;                         // Index of starting byte of current value in swaged buffer
        size_t end_byte   = (bit_index + bits_per_value) / 8;      // Index of ending byte of current value in swaged buffer

        uint8_t bit_offset = bit_index % 8;                        // Index of current bit in byte [0-7]
        uint8_t end_byte_bits = (bit_index + bits_per_value) % 8;  // Number of bits in end_byte occupied by current value

        int8_t right_shifts = 8 - bits_per_value - bit_offset;

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
        buf_out[i] = intermed;

        bit_index += bits_per_value;
    }

    return 0;
}
