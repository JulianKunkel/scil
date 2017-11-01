#include <stdio.h>
#include <assert.h>
#include <math.h>

#include <algo-quantize.h>
#include <scil-quantizer.h>
#include <scil-util.h>


//Supported datatypes: float double
// Repeat for each data type

int scil_quantize_compress_<DATATYPE>(const scil_context_t* ctx,
                                      int64_t* restrict dest,
                                      size_t* restrict out_size,
                                      <DATATYPE>*restrict source,
                                      const scil_dims_t* dims)
{
    size_t count = scil_dims_get_count(dims);

    <DATATYPE> minimum, maximum;
    scilU_find_minimum_maximum_<DATATYPE>(source, count, &minimum, &maximum);

    uint8_t bits_per_value = scil_calculate_bits_needed_<DATATYPE>(minimum, maximum, ctx->hints.absolute_tolerance, 0, NULL);
    if (bits_per_value > 64)
        return 1; // Quantizing would result in values bigger than UINT64_MAX

    *(double*)dest = (double)minimum;
    ++dest;

    *(double*)dest = ctx->hints.absolute_tolerance;
    ++dest;

    *out_size = 16;
    *out_size += count * sizeof(int64_t);

    char value[2];
    snprintf(value, 2, "%u", bits_per_value);
    scilU_dict_put(ctx->pipeline_params, "bits_per_value", value);

    return scil_quantize_buffer_minmax_<DATATYPE>((uint64_t*)dest, source, count, ctx->hints.absolute_tolerance, minimum, maximum);
}

int scil_quantize_decompress_<DATATYPE>(<DATATYPE>*restrict dest,
                                        scil_dims_t* dims,
                                        int64_t*restrict source,
                                        const size_t in_size)
{
    double minimum = *(double*)source;
    ++source;

    double abstol  = *(double*)source;
    ++source;

    return scil_unquantize_buffer_<DATATYPE>(dest, (uint64_t*)source, scil_dims_get_count(dims), abstol, minimum);
}
// End repeat

scilU_algorithm_t algo_quantize = {
    .c.Ctype = {
        CREATE_INITIALIZER(scil_quantize)
    },
    "quantize",
    9,
    SCIL_COMPRESSOR_TYPE_DATATYPES_CONVERTER,
    1
};
