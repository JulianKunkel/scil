#include <scil-quantizer.h>

#include <math.h>

#include <scil-errors.h>

static double find_minimum_double(const double* const buffer, const size_t count){

    double min = INFINITY;

    for(size_t i = 0; i < count; ++i){
        if(buffer[i] < min){
            min = buffer[i];
        }
    }

    return min;
}
static float find_minimum_float(const float* const buffer, const size_t count){

    float min = INFINITY;

    for(size_t i = 0; i < count; ++i){
        if(buffer[i] < min){
            min = buffer[i];
        }
    }

    return min;
}

static uint64_t scil_quantize_value_double(const double value,
                                           const double minimum,
                                           const double absolute_tolerance){

    return (uint64_t)round((value - minimum) / (2 * absolute_tolerance));
}
static uint64_t scil_quantize_value_float(const float value,
                                          const float minimum,
                                          const double absolute_tolerance){

    return (uint64_t)roundf((value - minimum) / (float)(2 * absolute_tolerance));
}

int scil_quantize_buffer_double(const double* const restrict buf_in,
                                uint64_t* const restrict buf_out,
                                const size_t count,
                                const double absolute_tolerance){

    double minimum = find_minimum_double(buf_in, count);

    for(size_t i = 0; i < count; ++i){
        buf_out[i] = scil_quantize_value_double(buf_in[i], minimum, absolute_tolerance);
    }

    return 0;
}
int scil_quantize_buffer_float(const float* const restrict buf_in,
                                uint64_t* const restrict buf_out,
                                const size_t count,
                                const double absolute_tolerance){

    float minimum = find_minimum_float(buf_in, count);

    for(size_t i = 0; i < count; ++i){
        buf_out[i] = scil_quantize_value_float(buf_in[i], minimum, absolute_tolerance);
    }

    return 0;
}
