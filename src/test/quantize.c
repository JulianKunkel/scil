#include <scil-quantizer.c>

#include <math.h>
#include <time.h>

#include <scil-util.h>

int main(void){

    const size_t count = 1000;

    double*   buf_in  = (double*)SAFE_MALLOC(count * sizeof(double));
    uint64_t* buf_out = (uint64_t*)SAFE_MALLOC(count * sizeof(uint64_t));
    double*   buf_end = (double*)SAFE_MALLOC(count * sizeof(double));

    srand((unsigned)time(NULL));

    printf("#Testing scil-quantizer\n\n");
    for(uint16_t i = 1; i < 100; ++i){

        for(uint16_t j = 0; j < count; ++j){
            buf_in[j] = -1000000.0 + ((double)rand() / (double)RAND_MAX) * 2000000.0;
        }

        double minimum, maximum;
        scil_find_minimum_maximum_double(buf_in, count, &minimum, &maximum);

        printf("#Cycle %d with following attributes:\n");
        printf("#Absolute tolerance: %f, Minimum: %f, Maximum: %f\n", absolute_tolerance, minimum, maximum);

        double absolute_tolerance = pow(2.0, -i);

        uint64_t bits_needed = scil_calculate_bits_needed(minimum, maximum, absolute_tolerance);
        uint8_t ret = scil_quantize_buffer_minmax_double(buf_out, buf_in, count, absolute_tolerance);

        if(bits_needed > 64){
            // Failure testing
            printf("#Needed bits for quantized values > 64.\n");

            if(ret != 1){
                printf("#FAILURE: scil_quantize_buffer did the quantization even though quantized values need more than 64 bits.");
                return 1;
            }
            printf("\n");
        }
        else{
            // Success testing

            printf("#Needed bits for quantized values <= 64.\n\n");

            scil_unquantize_buffer_double(buf_end, buf_out, count, absolute_tolerance, minimum);

            printf("#Values before and after a quant-unquant-cycle.\n");
            printf("#before,after\n");
            for(uint16_t j = 0; j < count; ++j){
                printf("%f,%f\n", buf_in[j], buf_end[j])
                if( fabs(buf_in[j] - buf_end[j]) > absolute_tolerance ){
                    printf("#FAILURE: value at index %d was after quantizing not in the tolerated error margin.\n", j);
                    return 1;
                }
            }
            printf("\n");
        }
    }

    printf("#SUCCESS");

    return 0;
}
