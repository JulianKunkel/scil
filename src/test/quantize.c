#include <scil-quantizer.h>

#include <scil-util.h>

int main(void){

    const size_t count = 1000;

    double*   buf_in  = (double*)SAFE_MALLOC(count * sizeof(double));
    uint64_t* buf_out = (uint64_t*)SAFE_MALLOC(count * sizeof(uint64_t));
    double*   buf_end = (double*)SAFE_MALLOC(count * sizeof(double));



    double minimum, maximum;
    scil_find_minimum_maximum_double(buf_in, count, &minimum, &maximum);

    scil_quantize_buffer_minmax_double(buf_out, buf_in, count, absolute_tolerance, minimum, maximum);
    scil_unquantize_buffer_double(buf_end, buf_out, count, absolute_tolerance, minimum)

    return 0;
}
