#include <scil-packer.h>

static uint8_t start_mask[9] = {
    255  //0b11111111
    127, //0b01111111
    63,  //0b00111111
    31,  //0b00011111
    15,  //0b00001111
    7,   //0b00000111
    3,   //0b00000011
    1,   //0b00000001
    0,   //0b00000000
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

int scil_swage(byte* const restrict buf_out,
               const uint64_t* const restrict buf_in,
               const size_t count,
               const uint8_t bits_per_value)
{
    for(size_t i = 0, size_t bit_index = 0; i < count; ++i, bit_index+=bits_per_value)
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
        for(uint8_t k = 1, uint8_t j = start_byte + 1; j <= end_byte; ++j, ++k)
        {
            buf_out[j] = right_shift_64(buf_in[i], right_shifts - k * 8);
        }
    }

    return 0;
}

int scil_unswage(uint64_t* const restrict buf_out,
                 const byte* const restrict buf_in,
                 const size_t count,
                 const uint8_t bits_per_value)
{
    for(size_t i = 0, size_t bit_index = 0; i < count; ++i, bit_index+=bits_per_value)
    {
        size_t start_byte = bit_index / 8;
        size_t end_byte   = (bit_index + bits_per_value) / 8;
        uint8_t bit_offset = bit_index % 8;

        int8_t right_shifts = 8 - bits_per_value - bit_offset;

        if(start_byte == end_byte){
            buf_out[i] = buf_in[start_byte]
        }

        byte intermed = buf_in[start_byte] & start_mask[bit_offset];
        if (bit_offset + bits_per_value < 8)
            intermed &= end_mask[bit_offset + bits_per_value];

        right_shift_8(intermed, right_shifts);

        /*
        // Read from first byte

        // Set remaining bits in start_byte to 0
        buf_out[start_byte] &= end_mask[bit_offset];
        // Write as much bits of number as possible to remaining bits
        buf_out[start_byte] |= (byte)(buf_in[i] >> right_shifts);

        // Write to following bytes
        for(uint8_t k = 1, uint8_t j = start_byte + 1; j <= end_byte; ++j, ++k)
        {
            buf_out[j] = (byte)(buf_in[i] >> (right_shifts - k * 8));
        }
        */
    }

    return 0;
}
