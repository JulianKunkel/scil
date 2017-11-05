#include <stdio.h>
#include <time.h>

#include <scil.h>
#include <scil-util.h>

/*  We test some extreme numbers like biggest, smallest, see below
      with some given rel. tolerances to see if they match
      in combination with some given finest absolute tolerances

    For IEEE 754 standard see
    https://en.wikipedia.org/wiki/IEEE_754
    https://en.wikipedia.org/wiki/Double-precision_floating-point_format
*/

#define count 3
double input_values[count];

void init() {
    int i = -1;
    //datatype_cast_double f;

/*  No sense in testing +/-0, +/-infinity, NaN
    as the resulting rel. error is undefined
    but we already tested them in sigbits tests
*/

/*  There is the minimal normal positive value
    having exponent -1022, encoded as 1
    and mantissa 0, meaning 1.0

    Compression should always be precise
    as mantissa 0 should not cause any overflow
    
    We already tested this for sigbits, it's ok for reltol then

    http://www.binaryconvert.com/result_double.html?hexadecimal=0010000000000000
*/

/*  Test the minimal denormalized positive value
    having exponent -1022, encoded as 0
    and mantissa 0.0 + 2^-52

    Rounding up only happens on 52 sigbits (you could work out the
    corresponding rel. tolerance), still denormalized
    We then expect the rel. error to be within tolerance
    
    Rounding down causes min. denormalized value to be 0.0
    This would only be a problem if reltol_finest_abstol is 0
    If it is at least this minimal denormalized positive value 
    (couldn't be smaller), than result 0 is a valid absolute error.

    http://www.binaryconvert.com/result_double.html?hexadecimal=0000000000000001
*/
    input_values[++i] = 4.94065645841246544176568792868E-324;

/*  Also test maximal denormalized positive value
    having exponent -1022, encoded as 0
    and mantissa 0.0 + (1 − 2^−52)

    Rounding up for any compression level < 53 sigbits
    Result should be minimal normal positive value

    http://www.binaryconvert.com/result_double.html?hexadecimal=000FFFFFFFFFFFFF
*/
    input_values[++i] = 2.22507385850720088902458687609E-308;

/*  And test maximal normalized positive value
    having exponent 1023, encoded as 2046
    and mantissa 1.0 + (1 − 2^−52)

    Rounding up for any compression level < 53 sigbits
    causes overflow and sets exponent to 1024, meaning infinity

    This is accepted, so we do not test here

    http://www.binaryconvert.com/result_double.html?hexadecimal=7FEFFFFFFFFFFFFF
*/
    //input_values[++i] = 1.79769313486231570814527423732E308;

/*  Test 0
*/
    input_values[++i] = 0.0;
}

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

int test_reltol(double tolerance, double finest) {
    scil_user_hints_t hints;
    scil_user_hints_initialize(&hints);
    hints.relative_tolerance_percent = tolerance;
    hints.relative_err_finest_abs_tolerance = finest;
    hints.force_compression_methods = "3";
    printf("#Testing sigbits algorithm with 'reltol' = %g %%, 'finest' = %g\n", tolerance, finest);

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

    for(size_t i = 0; i < count; ++i){
        buffer_in[i] = input_values[i];
    }

    size_t out_size;
    ret = scil_compress(buffer_out, compressed_size, buffer_in, &dims, &out_size, context);
    scil_decompress(SCIL_TYPE_DOUBLE, buffer_end, &dims, buffer_out, out_size, buffer_tmp);

    if(ret != SCIL_NO_ERR){
      return -1;
    }

    int errors = 0;
    printf("#Input value,Value after comp-decomp,Rel. error %%, Abs. error,Status\n");
    for (size_t i = 0; i < count; ++i) {
        double rel_err = rel_error_percent(buffer_in[i], buffer_end[i]);
        double abs_err = abs_error(buffer_in[i], buffer_end[i]);
        printf("%12.4e,%12.4e,%12.4e,%12.4e", buffer_in[i], buffer_end[i], rel_err, abs_err);
        if (rel_err <= tolerance || abs_err <= finest) {
            printf(",Ok\n");
        }
        else {
            printf(",Error\n");
            errors++;
        }
    }

    free(buffer_in);
    free(buffer_out);
    free(buffer_tmp);
    free(buffer_end);

    //scil_destroy_context(context);

    return errors;
}

int main(void) {
    init();
    int errors = 0;
    // You could test finest = 0 to see problematic cases we accept
    // errors += test_reltol(50, 0);
    errors += test_reltol(50, 4.94065645841246544176568792868E-324);
    errors += test_reltol(50, 1);
    // reltol = 4e-14 will result in 52 sigbits
    // errors += test_reltol(4e-14, 0);
    errors += test_reltol(4e-14, 4.94065645841246544176568792868E-324);
    errors += test_reltol(4e-14, 1);
    return errors;
}
