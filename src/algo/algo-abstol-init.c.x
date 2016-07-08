#ifndef SCIL_ABSTOL_INITIALZIER
#define SCIL_ABSTOL_INITIALZIER

#define SCIL_ABSTOL_HEADER_SIZE 18
#define SCIL_SMALL 0.00000000000000000000001

static uint64_t round_up_byte(const uint64_t bits){

    uint8_t a = bits % 8;
    if(a == 0)
        return bits / 8;
    return 1 + (bits - a) / 8;
}

static void read_header(const byte* source,
                        size_t* const source_size,
                        double* const minimum,
                        double* const absolute_tolerance,
                        uint8_t* const bits_per_value){

    *minimum = *((double*)(source));
    source += 8;
    *source_size -= 8;

    *absolute_tolerance = *((double*)(source));
    source += 8;
    *source_size -= 8;

    *bits_per_value = *source;
    source += 1;
    *source_size -= 1;
}

static void write_header(byte* dest,
                         double minimum,
                         double absolute_tolerance,
                         uint8_t bits_per_value){

    *((double*)dest) = minimum;
    dest += 8;

    *((double*)dest) = absolute_tolerance;
    dest += 8;

    *dest = bits_per_value;
    dest += 1;
}

scil_compression_algorithm algo_abstol = {
    .c.DNtype = {
        scil_abstol_compress_float,
        scil_abstol_decompress_float,
        scil_abstol_compress_double,
        scil_abstol_decompress_double,
    },
    "abstol",
    1,
    SCIL_COMPRESSOR_TYPE_DATATYPES,
    1
};

#endif
