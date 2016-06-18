#include <scil-packer.h>

int scil_pack(byte* const restrict buf_out,
              const uint64_t* const restrict buf_in,
              const size_t count,
              const uint8_t bits_per_value)
{
    size_t bit_index = 0;

    for(size_t i = 0; i < count; ++i){
        size_t start_byte = bit_index / 8;
        size_t end_byte   = (bit_index + bits_per_value) / 8;
        uint8_t bit_offset = bit_index % 8;

        if(start_byte == end_byte){
            buf_out[start_byte] |= buf_in[i] << (8 - bit_offset - bits_per_value);
        } else if(start_byte + 1 == end_byte){
            buf_out[start_byte] |= buf_in[i] << (8 - bit_offset - bits_per_value);
        } else {

        }

        bit_index += bits_per_value;
    }

    return 0;
}

int scil_unpack(uint64_t* const restrict buf_out,
                const byte* const restrict buf_in,
                const size_t count,
                const uint8_t bits_per_value){


    return 0;
}
