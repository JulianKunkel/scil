#include <scil-quantizer.c>

#include <math.h>
#include <time.h>

#include <scil-util.h>

int main(void){

    const size_t count = 1000000;

    double*   buf_in  = (double*)scilU_safe_malloc(count * sizeof(double));
    uint64_t* buf_out = (uint64_t*)scilU_safe_malloc(count * sizeof(uint64_t));
    double*   buf_end = (double*)scilU_safe_malloc(count * sizeof(double));

    srand((unsigned)time(NULL));

    printf("#Testing scil-quantizer\n\n");
    for(uint16_t i = 1; i < 55; ++i){

        for(size_t j = 0; j < count; ++j){
            buf_in[j] = 1000.0 * ( -1.0 + ((double)rand() / (double)RAND_MAX) * 2.0 );
        }

        double absolute_tolerance = pow(2.0, -i);

        double minimum, maximum;
        scilU_find_minimum_maximum_double(buf_in, count, &minimum, &maximum);

        printf("#Cycle %d with following attributes:\n", i);
        printf("#Absolute tolerance: %.20f, Minimum: %+.20f, Maximum: %+.20f\n", absolute_tolerance, minimum, maximum);

        uint64_t bits_needed = scil_calculate_bits_needed_double(minimum, maximum, absolute_tolerance, 0, NULL);

        printf("#Bits needed per value: %lu\n", bits_needed);

        /*
        datatype_cast_double max_union, max_union_m1;
        max_union.f = maximum;
        max_union_m1.f = maximum;
        max_union_m1.p.mantissa -= 1;

        double max_prec = max_union.f - max_union_m1.f;
        */

        double d_max_prec = maximum * pow(10.0, -15.0);
        printf("#Maximum guaranteed double precision: %.20f\n", d_max_prec);

        if(d_max_prec > absolute_tolerance){
            printf("#!!!Risk of insufficient double precision!!!\n");
            printf("#Error tol.: %.20f\n", absolute_tolerance );
            printf("#Max. prec.: %.20f\n", d_max_prec);
        }

        printf("#Checking values before and after a quant-unquant-cycle.\n");

        uint8_t ret = scil_quantize_buffer_minmax_double(buf_out, buf_in, count, absolute_tolerance, minimum, maximum);

        if(bits_needed > 53){
            // Failure testing
            printf("#Needed bits for quantized values > 53.\n");

            if(ret != SCIL_EINVAL){
                printf("#FAILURE: scil_quantize_buffer did the quantization even though quantized values need more than 53 bits.\n");
            }
            printf("\n");
        }else{
            // Success testing
            printf("#Needed bits for quantized values <= 53.\n");

            scil_unquantize_buffer_double(buf_end, buf_out, count, absolute_tolerance, minimum);

            //printf("#before,after\n");
            for(size_t j = 0; j < count; ++j){
                //printf("%+.20f,%+.20f\n", buf_in[j], buf_end[j]);
                double delta = fabs(buf_in[j] - buf_end[j]);
                double error = (delta > absolute_tolerance) ? delta - absolute_tolerance : 0;
                if( error > FLT_FINEST_SUB_double*10 ){
                    printf("#FAILURE: value at index %lu was after quantizing not in the tolerated error margin.\n", j);
                    printf("#Before: %+.20f == %lu\n", buf_in[j], buf_out[j]);
                    printf("#After:  %+.20f\n", buf_end[j]);
                    printf("#Difference: %.20f -- error exceeds by: %.20f\n", delta, error);
                    printf("#Error tol.: %.20f\n", absolute_tolerance );
                    printf("#Max. prec.: %.20f\n", d_max_prec);
                    return 1;
                }
            }
            printf("\n");
        }
    }

    printf("#SUCCESS\n");

    return 0;
}
