#include <stdio.h>
#include <time.h>

#include <scil.h>
#include <scil-util.h>

/*  We test some extreme numbers like NaN, Infinity, 0, see below
      with 1 sigbit to watch the maximum rounding effect
      with 2 sigbits as next step
      with max number of sigbits, as we expect no compression, hence same value
      (don't know if in this case compression is called at all)
      with max number - 1, as here we can define expected rounding results for some values
      (and it's the extreme case where compression happens)

    Remember x sigbits =
      1   not stored bit for fixed 1.0 + stored mantissa
      x-1 real mantissa bits

    We also know the sigbit algorithm should round
      to lower value for input <  (lower + upper)/2
      to upper value for input >= (lower + upper)/2
    as this minimizes the resulting relative error.

    For IEEE 754 standard see
    https://en.wikipedia.org/wiki/IEEE_754
    https://en.wikipedia.org/wiki/Double-precision_floating-point_format
*/

#define count 9
double input_values[count];
double expected_1bit[count];
double expected_2bit[count];
double expected_pre2max_bits[count];
double expected_premax_bits[count];
double *expected_max_bits = input_values;

void init() {
    int i = -1;
    datatype_cast_double f;

/*  There are two signed representations of 0
    having exponent -1022, encoded as 0
      this is called denormalized
      we do NOT have fixed 1.0 + mantissa but 0.0 + mantissa
      same exponent is used as with encoding 1
      just change from normalized to denormalized
    having mantissa 0
      meaning 0.0 + mantissa
    sign bit 0(+) or 1(-)

    Compressing this would always lead to rounding down / truncation
    no overflow case to check here
*/
    input_values[++i] = 0.0;
    expected_1bit[i] = input_values[i];
    expected_2bit[i] = input_values[i];
    expected_pre2max_bits[i] = input_values[i];
    expected_premax_bits[i] = input_values[i];

    input_values[++i] = -0.0;
    expected_1bit[i] = input_values[i];
    expected_2bit[i] = input_values[i];
    expected_pre2max_bits[i] = input_values[i];
    expected_premax_bits[i] = input_values[i];

    printf("#Test: %e %s %e\n", input_values[i-1],
        input_values[i-1] == input_values[i] ? "==" : "!=",
        input_values[i]);

/*  Now test the minimal normal positive value
    having exponent -1022, encoded as 1
    and mantissa 0, meaning 1.0

    Compression should always be precise
    as mantissa 0 should not cause any overflow

    http://www.binaryconvert.com/result_double.html?hexadecimal=0010000000000000
*/
    f.p.exponent = 1; //2.22507385850720088902458687609E-308;
    f.p.mantissa = 0;
    f.p.sign = 0;
    input_values[++i] = f.f;
    expected_1bit[i] = input_values[i];
    expected_2bit[i] = input_values[i];
    expected_pre2max_bits[i] = input_values[i];
    expected_premax_bits[i] = input_values[i];

/*  Now test the minimal denormalized positive value
    having exponent -1022, encoded as 0
    and mantissa 0.0 + 2^-52

    Rounding up only happens on 52 sigbits, still denormalized

    TODO Rounding down causes min. denormalized value to be 0.0
    TODO This ruins reltol interpretation of sigbits!
    TODO Is this the best we can do? Give some better definition!

    http://www.binaryconvert.com/result_double.html?hexadecimal=0000000000000001
*/
    input_values[++i] = 4.94065645841246544176568792868E-324;
    expected_1bit[i] = 0.0;
    expected_2bit[i] = 0.0;
    expected_pre2max_bits[i] = 0.0;
    expected_premax_bits[i] = input_values[i] * 2.0;

/*  Also test maximal denormalized positive value
    having exponent -1022, encoded as 0
    and mantissa 0.0 + (1 − 2^−52)

    Rounding up for any compression level < 53 sigbits
    Result should be minimal normal positive value

    http://www.binaryconvert.com/result_double.html?hexadecimal=000FFFFFFFFFFFFF
*/
    input_values[++i] = 2.22507385850720088902458687609E-308;
    expected_1bit[i] = 2.22507385850720138309023271733E-308;
    expected_2bit[i] = expected_1bit[i];
    expected_pre2max_bits[i] = expected_1bit[i];
    expected_premax_bits[i] = expected_1bit[i];

/*  There is +/- infinity
    having maximal exponent 1024, encoded as 2047
      this exponent is not used for number range
      valid numbers have exp. 1023 max because of this infinity / NaN rule
    having mantissa 0
    sign bit is used

    Compression should always be precise
    as mantissa 0 should not cause any overflow
*/
    input_values[++i] = 1.0/0.0;
    expected_1bit[i] = input_values[i];
    expected_2bit[i] = input_values[i];
    expected_pre2max_bits[i] = input_values[i];
    expected_premax_bits[i] = input_values[i];

    input_values[++i] = 1.0/(-0.0);
    expected_1bit[i] = input_values[i];
    expected_2bit[i] = input_values[i];
    expected_pre2max_bits[i] = input_values[i];
    expected_premax_bits[i] = input_values[i];

    printf("#Test: %e %s %e\n", input_values[i-1],
        input_values[i-1] == input_values[i] ? "==" : "!=",
        input_values[i]);

/*  There is NaN (not a number)
    having maximal exponent 1024, encoded as 2047
      this exponent is not used for number range
      valid numbers have exp. 1023 max because of this infinity / NaN rule
    having any mantissa != 0
    having any sign bit

    Here we must check if mantissa is interpreted as number
    and hence may be rounded up, causing exponent to overflow.
    Such problem could be fixed to just stay with any NaN

    With 1 sigbit it's impossible to have mantissa != 0
    so we can't expect NaN but only infinity.
    Turns out the result is -infinity, we accept.

    0.0/0.0 undefined, causes NaN but don't know which bitmask

    Also force maximum bitmask, which is NaN
    and is big/little endian independent
    and would cause overflow if mantissa handled as number
    http://www.binaryconvert.com/result_double.html?hexadecimal=FFFFFFFFFFFFFFFF
*/
    input_values[++i] = 0.0/0.0;
    expected_1bit[i] = -1.0/0.0;
    expected_2bit[i] = input_values[i];
    expected_pre2max_bits[i] = input_values[i];
    expected_premax_bits[i] = input_values[i];

    unsigned long bitmask = 0xFFFFFFFFFFFFFFFF;
    input_values[++i] = *((double*)(&bitmask));
    expected_1bit[i] = -1.0/0.0;
    expected_2bit[i] = input_values[i];
    expected_pre2max_bits[i] = input_values[i];
    expected_premax_bits[i] = input_values[i];

    printf("#Test: %e[%d] %s %e[%d]\n", input_values[i-1], i-1,
        input_values[i-1] == input_values[i-1] ? "==" : "!=",
        input_values[i-1], i-1);
    printf("#Test: %e[%d] %s %e[%d]\n", input_values[i-1], i-1,
        input_values[i-1] == input_values[i] ? "==" : "!=",
        input_values[i], i);
    printf("#Test: %e[%d] %s %e[%d]\n", input_values[i], i,
        input_values[i] == input_values[i] ? "==" : "!=",
        input_values[i], i);
}

double expected_value(char sigbits, size_t i) {
    switch(sigbits) {
        case 1:
            return expected_1bit[i];
        case 2:
            return expected_2bit[i];
        // Remember: sigbits = mantissa bits + 1
        case MANTISSA_LENGTH_DOUBLE - 1:
            return expected_pre2max_bits[i];
        case MANTISSA_LENGTH_DOUBLE:
            return expected_premax_bits[i];
        case MANTISSA_LENGTH_DOUBLE + 1:
            return expected_max_bits[i];
    }
}

int same_value(double expected, double test) {
    if (expected != expected) {
        // expect NaN, looks strange but is correct
        return test != test;
    }
    return expected == test;
}

int test_sigbits(char sigbits) {
    scil_user_hints_t hints;
    scil_user_hints_initialize(&hints);
    hints.significant_bits = sigbits;
    hints.force_compression_methods = "3";
    printf("#Testing sigbit algorithm with hint 'number of sigbits' = %d\n", sigbits);

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
    printf("#Input value,Expected value,Value after comp-decomp,Status\n");
    for (size_t i = 0; i < count; ++i) {
        double expected = expected_value(sigbits, i);
        printf("%12.4e,%12.4e,%12.4e", buffer_in[i], expected, buffer_end[i]);
        if (same_value(expected, buffer_end[i])) {
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
    errors += test_sigbits(1);
    errors += test_sigbits(2);
    // Remember: sigbits = mantissa bits + 1
    errors += test_sigbits(MANTISSA_LENGTH_DOUBLE-1);
    if(test_sigbits(MANTISSA_LENGTH_DOUBLE) > 6) errors++;
    if(test_sigbits(MANTISSA_LENGTH_DOUBLE + 1) != -1){
      errors++;
    }
    return errors;
}
