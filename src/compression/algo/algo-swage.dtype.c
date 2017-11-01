
#include <algo/algo-swage.h>
#include <scil-quantizer.h>
#include <scil-util.h>

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

//Supported datatypes: int8_t int16_t int32_t int64_t
// Repeat for each data type

// Can do negative right shifts!! (left shifts)
static byte right_shift_64_<DATATYPE>(<DATATYPE> value, int amount)
{
    return (byte)(amount >= 0 ? value >> amount : value << -amount);
}
static <DATATYPE> right_shift_8_<DATATYPE>(byte value, int amount)
{
    return (amount >= 0 ? ((<DATATYPE>)value) >> amount : ((<DATATYPE>)value) << -amount);
}

static int scil_swage_<DATATYPE>(byte* restrict buf_out,
                                 const <DATATYPE>* restrict buf_in,
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
        buf_out[start_byte] |= right_shift_64_<DATATYPE>(buf_in[i], right_shifts);

        // Write to following bytes
        uint8_t k = 1;
        for(size_t j = start_byte + 1; j <= end_byte; ++j)
        {
            buf_out[j] = right_shift_64_<DATATYPE>(buf_in[i], right_shifts - k * 8);
            ++k;
        }

        bit_index += bits_per_value;
    }

    return 0;
}

static int scil_unswage_<DATATYPE>(<DATATYPE>* restrict buf_out,
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
        uint64_t intermed = right_shift_8_<DATATYPE>(buf_in[start_byte] & start_mask[bit_offset], right_shifts); // Masks away first value-unrelated bits in start_byte and shifts related bits to final position
        // Read from intermediate bytes
        uint8_t k = 1;
        for(size_t j = start_byte + 1; j < end_byte; ++j)
        {
            intermed |= right_shift_8_<DATATYPE>(buf_in[j], right_shifts + k * 8); // Shifts whole byte to final position and applies it
            ++k;
        }
        // Read from end_byte
        if(start_byte != end_byte)
        {
            intermed |= right_shift_8_<DATATYPE>(buf_in[end_byte], 8 - end_byte_bits); // Shifts out unrelated end bits in end_byte and applies value
        }

        // Write to output buffer
        buf_out[i] = intermed;

        bit_index += bits_per_value;
    }

    return 0;
}

int scil_swage_compress_<DATATYPE>(const scil_context_t* ctx,
                                   byte* restrict dest,
                                   size_t* restrict out_size,
                                   <DATATYPE>*restrict source,
                                   const scil_dims_t* dims)
{
    scilU_dict_element_t* elem = scilU_dict_get(ctx->pipeline_params, "bits_per_value");
    uint8_t bits_per_value;
    size_t count = scil_dims_get_count(dims);
    if( elem ){
      char* bpv_str = elem->value;
      bits_per_value = strtol(bpv_str, (char**)NULL, 10);
    }else{
      // we determine the value new based on min / max
      <DATATYPE> minimum, maximum;
      scilU_find_minimum_maximum_<DATATYPE>(source, count, &minimum, &maximum);

    }

    if (scil_swage_<DATATYPE>(dest, source, count, bits_per_value))
    {
        return 1;
    }

    return 0;
}

int scil_swage_decompress_<DATATYPE>(<DATATYPE>*restrict dest,
                                     scil_dims_t* dims,
                                     byte* restrict source,
                                     const size_t in_size)
{
    uint8_t bits_per_value = 0;


    if (scil_unswage_<DATATYPE>(dest, source, scil_dims_get_count(dims), bits_per_value))
    {
        return 1;
    }

    return 0;
}
// End repeat

scilU_algorithm_t algo_swage = {
    .c.DNtype = {
        CREATE_INITIALIZER(scil_swage)
    },
    "swage",
    10,
    SCIL_COMPRESSOR_TYPE_DATATYPES,
    1
};
