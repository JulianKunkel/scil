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
    if (lo == 0.0) // may be -0.0, make it +0.0
        return 1.0/0.0;
    return ((hi - lo) / lo) * 100.0;
}

double abs_error(double input, double output) {
    double err = output - input;
    err = max(err, -err);
    return err;
}

int test_allquant(size_t count, double reltol, double finest, double abstol, float min_value, float max_value) {

    scil_user_hints_t hints;
    scil_user_hints_initialize(&hints);
    hints.relative_tolerance_percent = reltol;
    hints.relative_err_finest_abs_tolerance = finest;
    hints.absolute_tolerance = abstol;
    hints.force_compression_methods = "12";
    //printf("#Testing allquant algorithm with hint 'reltol' = %f %%, 'finest' = %f, 'abstol' = %f\n", reltol, finest, abstol);

    scil_context_t* context;
    int ret = scil_context_create(&context, SCIL_TYPE_FLOAT, 0, NULL, &hints);
    if(ret != SCIL_NO_ERR){
        //printf("#Error on scil_context_create(): %s\n", scil_error_get_message(ret));
        return -2;
    }

    scil_dims_t dims;
    scil_dims_initialize_1d(&dims, count);

    size_t uncompressed_size = scil_dims_get_size(&dims, SCIL_TYPE_FLOAT);
    size_t compressed_size   = scil_get_compressed_data_size_limit(&dims, SCIL_TYPE_FLOAT);

    float* buffer_in  = (float*)malloc(uncompressed_size);
    byte* buffer_out   = (byte*)malloc(compressed_size);
    byte* buffer_tmp   = (byte*)malloc(compressed_size / 2);
    float* buffer_end = (float*)malloc(uncompressed_size);

    float delta = (max_value - min_value) / (count - 1);
    for(size_t i = 0; i < count; ++i){
        buffer_in[i] = min_value + delta * i;
    }

    size_t out_size;
    ret = scil_compress(buffer_out, compressed_size, buffer_in, &dims, &out_size, context);
    if(ret != SCIL_NO_ERR){
        //printf("#Error on scil_compress(): %s\n", scil_error_get_message(ret));
        return -1;
    }
    ret = scil_decompress(SCIL_TYPE_FLOAT, buffer_end, &dims, buffer_out, out_size, buffer_tmp);
    if(ret != SCIL_NO_ERR){
        //printf("#Error on scil_decompress(): %s\n", scil_error_get_message(ret));
        return -1;
    }

    int errors = 0;
    double max_rel_error = 0.0;
    double max_finest_error = 0.0;
    double max_abs_error = 0.0;
    //printf("#Value, Value after comp-decomp, Relative error, Absolute error\n");
    for (size_t i = 0; i < count; ++i) {
        double rel_err = rel_error_percent(buffer_in[i], buffer_end[i]);
        double abs_err = abs_error(buffer_in[i], buffer_end[i]);
        //printf("%f,%f,%f,%f\n", buffer_in[i], buffer_end[i], rel_err, abs_err);
        if (!(
          (rel_err <= reltol || abs_err <= finest) &&
          (abs_err <= abstol)
          )) {
            errors++;
        }
        if (abs_err > finest) {
            max_rel_error = max(max_rel_error, rel_err);
        } else {
            max_finest_error = max(max_finest_error, abs_err);
        }
        max_abs_error = max(max_abs_error, abs_err);
    }

    printf("%7lu, %8.1f, %7.1f, %5.1f, %5.1f, %5.1f, %5.2f, %5.2f, %6.1f, %5.2f%%, %s\n",
        count, min_value, max_value,
        reltol, finest, abstol,
        max_rel_error, max_finest_error, max_abs_error,
        100.0 * out_size / uncompressed_size,
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
    printf("#Num-values, Min-value, Max-value, Reltol, Finest, Abstol, Max-rel-err, Max-finest-err, Max-abs-err, Compressed-size, Status\n");
    for(int num = 100; num <= 1000000; num *= 100) {
        errors += test_allquant(num, 10, 0.1, 20, 0, 100);
        errors += test_allquant(num, 10, 0.1, 20, 0, 1000);
        errors += test_allquant(num, 10, 0.1, 20, 0, 10000);
        errors += test_allquant(num, 30, 0.5, 200, 0, 100);
        errors += test_allquant(num, 30, 0.5, 200, 0, 1000);
        errors += test_allquant(num, 30, 0.5, 200, 0, 10000);
        errors += test_allquant(num, 10, 0.1, 20, -100, 100);
        errors += test_allquant(num, 10, 0.1, 20, -1000, 1000);
        errors += test_allquant(num, 10, 0.1, 20, -10000, 10000);
        errors += test_allquant(num, 30, 0.5, 200, -100, 100);
        errors += test_allquant(num, 30, 0.5, 200, -1000, 1000);
        errors += test_allquant(num, 30, 0.5, 200, -10000, 10000);
    }
    if (-1 != test_allquant(1000, 50, 20, 10, 0, 1000))
    {
        printf("Did not get SCIL_PRECISION_ERR on finest > abstol, Failed\n");
        errors++;
    }
    return errors;
}
