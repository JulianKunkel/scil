#include <stdio.h>
#include <time.h>

#include <scil.h>
#include <scil-util.h>
#include <scil-error.h>

double rel_error_percent(double input, double output) {
    // TODO Did not found a global function to calculate this
    // TODO How is the error definition if sign bit changed? Should not happen.
    if ((input < 0 && output > 0) || (input > 0 && output < 0))
        return 1.0/0.0; // Infinite error
    double in = max(input, -input);
    double out = max(output, -output);
    double hi = max(in, out);
    double lo = min(in, out);
    return 100.0 * (hi - lo) / lo;
}

int test_reltol(double tolerance) {
    size_t count = 1000;

    scil_user_hints_t hints;
    scil_user_hints_initialize(&hints);
    hints.relative_tolerance_percent = tolerance;
    //hints.significant_bits = 11;
    hints.force_compression_methods = "3";
    //printf("#Testing sigbit algorithm with hint 'relative tolerance' = %f %%\n", tolerance);

    scil_context_t* context;
    int ret = scil_context_create(&context, SCIL_TYPE_DOUBLE, 0, NULL, &hints);
    if(ret != SCIL_NO_ERR){
        //printf("#Error on scil_context_create(): %s\n", scil_error_get_message(ret));
        return -2;
    }

    scil_dims_t dims;
    scil_dims_initialize_1d(&dims, count);

    size_t uncompressed_size = scil_dims_get_size(&dims, SCIL_TYPE_DOUBLE);
    size_t compressed_size   = scil_get_compressed_data_size_limit(&dims, SCIL_TYPE_DOUBLE);

    double* buffer_in  = (double*)malloc(uncompressed_size);
    byte* buffer_out   = (byte*)malloc(compressed_size);
    byte* buffer_tmp   = (byte*)malloc(compressed_size / 2);
    double* buffer_end = (double*)malloc(uncompressed_size);

    srand((unsigned)time(NULL));
    for(size_t i = 0; i < count; ++i){
        buffer_in[i] = -100.0 + (double)rand()/RAND_MAX * 200.0;
    }

    size_t out_size;
    ret = scil_compress(buffer_out, compressed_size, buffer_in, &dims, &out_size, context);
    if(ret != SCIL_NO_ERR){
        //printf("#Error on scil_compress(): %s\n", scil_error_get_message(ret));
        return -1;
    }
    ret = scil_decompress(SCIL_TYPE_DOUBLE, buffer_end, &dims, buffer_out, out_size, buffer_tmp);
    if(ret != SCIL_NO_ERR){
        //printf("#Error on scil_decompress(): %s\n", scil_error_get_message(ret));
        return -1;
    }

    int errors = 0;
    double max_error = 0.0;
    //printf("#Value,Value after comp-decomp,Relative error\n");
    for (size_t i = 0; i < count; ++i) {
        double rel_err = rel_error_percent(buffer_in[i], buffer_end[i]);
        //printf("%f,%f,%f\n", buffer_in[i], buffer_end[i], rel_err);
        if (rel_err > tolerance) {
            errors++;
        }
        max_error = max(max_error, rel_err);
    }

    printf("%12.6g, %12.6g, %2d, %s\n", tolerance, max_error, 
        scilU_relative_tolerance_to_significant_bits(tolerance), 
        errors > 0 ? "Failed" : "Ok");

    free(buffer_in);
    free(buffer_out);
    free(buffer_tmp);
    free(buffer_end);

    //scil_destroy_context(context);

    return errors;
}

int main(void){
    int errors = 0;
    printf("#Rel tolerance %%, Maximum rel. error %% after comp-decomp, Sigbits, Status\n");
    errors += test_reltol(200.0);     // Same effect as 50.0
    errors += test_reltol(50.0);
    errors += test_reltol(49.9999);   // Same effect as 25.0
    errors += test_reltol(25.0001);   // Same effect as 25.0
    errors += test_reltol(1.0);
    errors += test_reltol(0.00001);
    for (double r = 0.000001; r > 4.0E-14; r /= 2.0)
        errors += test_reltol(r);
    errors += test_reltol(4.0E-14);   // 52 sigbits
    if(test_reltol(2.0E-14) != -1) {  // 53 sigbits, no compr., too precise
        errors++;
    }
    return errors;
}
