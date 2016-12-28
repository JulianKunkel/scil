#include <scil-quantizer.h>
#include <scil-error.h>

#include <assert.h>
#include <math.h>
#include <limits.h>

#define INFINITY_double INFINITY
#define INFINITY_float INFINITY

#define NINFINITY_double -INFINITY
#define NINFINITY_float -INFINITY

#define INFINITY_int8_t CHAR_MAX
#define NINFINITY_int8_t CHAR_MIN

#define INFINITY_int16_t SHRT_MAX
#define NINFINITY_int16_t SHRT_MIN

#define INFINITY_int32_t INT_MAX
#define NINFINITY_int32_t INT_MIN

#define INFINITY_int64_t LONG_MAX
#define NINFINITY_int64_t LONG_MIN

//Supported datatypes: int8_t int16_t int32_t int64_t float double
// Repeat for each data type

static uint64_t scil_quantize_value_<DATATYPE>(<DATATYPE> value,
                                               double absolute_tolerance,
                                               <DATATYPE> minimum){
    return (uint64_t) round( ((double) (value - minimum)) * absolute_tolerance );
}

static <DATATYPE> scil_unquantize_value_<DATATYPE>(uint64_t value,
                                                   double absolute_tolerance,
                                                   <DATATYPE> minimum){
    return minimum + (<DATATYPE>)(value * absolute_tolerance);
}

<DATATYPE> scil_find_minimum_<DATATYPE>(const <DATATYPE>* buffer,
                                        size_t count){
    assert(buffer != NULL);

    <DATATYPE> min = INFINITY_<DATATYPE>;

    for(size_t i = 0; i < count; ++i)
    {
        if(buffer[i] < min) { min = buffer[i]; }
    }

    return min;
}

<DATATYPE> scil_find_maximum_<DATATYPE>(const <DATATYPE>* buffer,
                                        size_t count){

    assert(buffer != NULL);

    <DATATYPE> max = NINFINITY_<DATATYPE>;

    for(size_t i = 0; i < count; ++i)
    {
        if(buffer[i] > max) { max = buffer[i]; }
    }

    return max;
}

void scil_find_minimum_maximum_<DATATYPE>(const <DATATYPE>* restrict buffer,
                                          size_t count,
                                          <DATATYPE>* minimum,
                                          <DATATYPE>* maximum){

    assert(buffer != NULL);
    assert(minimum != NULL);
    assert(maximum != NULL);

    <DATATYPE> min = INFINITY_<DATATYPE>;
    <DATATYPE> max = NINFINITY_<DATATYPE>;

    for(size_t i = 0; i < count; ++i){
        if (buffer[i] < min) { min = buffer[i]; }
        if (buffer[i] > max) { max = buffer[i]; }
    }

    *minimum = min;
    *maximum = max;
}

uint64_t scil_calculate_bits_needed_<DATATYPE>(<DATATYPE> minimum,
                                               <DATATYPE> maximum,
                                               double absolute_tolerance){
    return (uint64_t) ceil( log2( 1.0 + (double)(maximum - minimum) / absolute_tolerance ) );
}

#include <stdio.h>
int scil_quantize_buffer_minmax_<DATATYPE>(uint64_t* restrict dest,
                                           const <DATATYPE>* restrict source,
                                           size_t count,
                                           double absolute_tolerance,
                                           <DATATYPE> minimum,
                                           <DATATYPE> maximum){

    assert(dest != NULL);
    assert(source != NULL);

    if(scil_calculate_bits_needed_<DATATYPE>(minimum, maximum, absolute_tolerance) > 53){
        return SCIL_EINVAL; // Quantizing would result in values bigger than UINT64_MAX
    }
    double real_tolerance = (1 / 1.0) / absolute_tolerance;

    for(size_t i = 0; i < count; ++i){
        dest[i] = scil_quantize_value_<DATATYPE>(source[i], real_tolerance, minimum);
    }

    return SCIL_NO_ERR;
}

int scil_unquantize_buffer_<DATATYPE>(<DATATYPE>* restrict dest,
                                      const uint64_t* restrict source,
                                      size_t count,
                                      double absolute_tolerance,
                                      <DATATYPE> minimum){

    assert(dest != NULL);
    assert(source != NULL);

    double real_tolerance = 1.0 * absolute_tolerance;

    for(size_t i = 0; i < count; ++i){
        dest[i] = scil_unquantize_value_<DATATYPE>(source[i], real_tolerance, minimum);
    }

    return 0;
}

int scil_quantize_buffer_<DATATYPE>(uint64_t* restrict dest,
                                    const <DATATYPE>* restrict source,
                                    size_t count,
                                    double absolute_tolerance){

    assert(dest != NULL);
    assert(source != NULL);

    <DATATYPE> minimum, maximum;
    scil_find_minimum_maximum_<DATATYPE>(source, count, &minimum, &maximum);

    return scil_quantize_buffer_minmax_<DATATYPE>(dest, source, count, absolute_tolerance, minimum, maximum);
}
// End repeat
