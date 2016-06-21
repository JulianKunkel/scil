#include <scil-packer.h>
static uint8_t start_mask[9] = {
    0,   //0b00000000
    1,   //0b00000001
    3,   //0b00000011
    7,   //0b00000111
    15,  //0b00001111
    31,  //0b00011111
    63,  //0b00111111
    127, //0b01111111
    255  //0b11111111
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


static uint8_t get_bits(uint64_t value, uint8_t start_bit, uint8_t bit_count)
{
    return (uint8_t)((value << (64 - start_bit)) >> (64 - bit_count));
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

        uint8_t right_shifts = bits_per_value - 8 + bit_offset;

        // Write to first byte
        buf_out[start_byte] &= end_mask[bit_offset];
        buf_out[start_byte] |= (byte)(buf_in[i] >> right_shifts;

        // Write to following bytes
        for(uint8_t k = 1, uint8_t j = start_byte + 1; j <= end_byte; ++j, ++k){
            buf_out[j] = (byte)(buf_in[i] >> (right_shifts - 8 * k));
        }
    }

    return 0;
}

int scil_unswage(uint64_t* const restrict buf_out,
                const byte* const restrict buf_in,
                const size_t count,
                const uint8_t bits_per_value){


    return 0;
}
