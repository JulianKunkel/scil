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

int test_reltol(double min_value, double max_value, double tolerance, double finest) {
    size_t count = 1000;

    scil_user_hints_t hints;
    scil_user_hints_initialize(&hints);
    hints.relative_tolerance_percent = tolerance;
    hints.relative_err_finest_abs_tolerance = finest;
    hints.force_compression_methods = "3";

    scil_context_t* context;
    int ret = scil_context_create(&context, SCIL_TYPE_DOUBLE, 0, NULL, &hints);
    if(ret != SCIL_NO_ERR){
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

    double delta = (max_value - min_value) / (count - 1);
    for(size_t i = 0; i < count; ++i){
        buffer_in[i] = min_value + delta * i;
    }

    size_t out_size;
    ret = scil_compress(buffer_out, compressed_size, buffer_in, &dims, &out_size, context);
    if(ret != SCIL_NO_ERR){
        return -1;
    }
    ret = scil_decompress(SCIL_TYPE_DOUBLE, buffer_end, &dims, buffer_out, out_size, buffer_tmp);
    if(ret != SCIL_NO_ERR){
        return -1;
    }

    int errors = 0;
    double max_rel_error = 0.0;
    double max_finest_error = 0.0;
    for (size_t i = 0; i < count; ++i) {
        double rel_err = rel_error_percent(buffer_in[i], buffer_end[i]);
        double abs_err = abs_error(buffer_in[i], buffer_end[i]);
        if (rel_err > tolerance && abs_err > finest) {
            errors++;
            printf("%f --> %f, +/- %f (%f %%) \n", buffer_in[i], buffer_end[i], abs_err, rel_err);
        }
        if (abs_err > finest) {
            max_rel_error = max(max_rel_error, rel_err);
        } else {
            max_finest_error = max(max_finest_error, abs_err);
        }
    }

    /* You will see on compressed size the effect of having (almost) 0
       will include negative-most exponents, resulting in a high
       exponent_bit_count.
       When algorithm respects "finest" and stores smaller values
       like fill values this effect should be reduced.
    */
    printf("%7.2f, %7.2f, %7.2f, %7.2f, %7.2f, %7.4f, %7.2f, %s\n",
        min_value, max_value,
        tolerance, finest, max_rel_error, max_finest_error,
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
    printf("#Min_value, Max_value, Rel_tol_%%, Finest, Result Max_rel_err_%%, Result Finest, Compressed_size_%%, Status\n");
    errors += test_reltol(1, 1000, 10, 0);
    errors += test_reltol(0, 1000, 10, 0);
    errors += test_reltol(0, 1, 10, 0);
    errors += test_reltol(1, 1000, 10, 0.01);
    errors += test_reltol(0, 1000, 10, 0.01);
    errors += test_reltol(0, 1, 10, 0.01);
    errors += test_reltol(1, 1000, 10, 0.1);
    errors += test_reltol(0, 1000, 10, 0.1);
    errors += test_reltol(0, 1, 10, 0.1);
    return errors;
}
