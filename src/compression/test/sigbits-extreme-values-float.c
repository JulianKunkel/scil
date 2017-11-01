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
    https://en.wikipedia.org/wiki/Single-precision_floating-point_format
    https://www.h-schmidt.net/FloatConverter/IEEE754.html
*/

#define count 9
float input_values[count];
float expected_1bit[count];
float expected_2bit[count];
float expected_pre2max_bits[count];
float expected_premax_bits[count];
float *expected_max_bits = input_values;

void init() {
    int i = -1;

/*  There are two signed representations of 0
    having exponent -126, encoded as 0
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
    having exponent -126, encoded as 1
    and mantissa 0, meaning 1.0

    Compression should always be precise
    as mantissa 0 should not cause any overflow

    http://www.binaryconvert.com/result_float.html?hexadecimal=00800000
*/
    input_values[++i] = 1.17549435082228750796873653722E-38;
    expected_1bit[i] = input_values[i];
    expected_2bit[i] = input_values[i];
    expected_pre2max_bits[i] = input_values[i];
    expected_premax_bits[i] = input_values[i];

/*  Now test the minimal denormalized positive value
    having exponent -126, encoded as 0
    and mantissa 0.0 + 2^-23

    Rounding up only happens on 23 sigbits, still denormalized

    TODO Rounding down causes min. denormalized value to be 0.0
    TODO This ruins reltol interpretation of sigbits!
    TODO Is this the best we can do? Give some better definition!

    http://www.binaryconvert.com/result_float.html?hexadecimal=00000001
*/
    input_values[++i] = 1.40129846432481707092372958329E-45;
    expected_1bit[i] = 0.0;
    expected_2bit[i] = 0.0;
    expected_pre2max_bits[i] = 0.0;
    expected_premax_bits[i] = input_values[i] * 2.0;

/*  Also test maximal denormalized positive value
    having exponent -126, encoded as 0
    and mantissa 0.0 + (1 − 2^−23)

    Rounding up for any compression level < 24 sigbits
    Result should be minimal normal positive value

    http://www.binaryconvert.com/result_float.html?hexadecimal=007FFFFF
*/
    input_values[++i] = 1.17549421069244107548702944485E-38;
    expected_1bit[i] = 1.17549435082228750796873653722E-38;
    expected_2bit[i] = expected_1bit[i];
    expected_pre2max_bits[i] = expected_1bit[i];
    expected_premax_bits[i] = expected_1bit[i];

/*  There is +/- infinity
    having maximal exponent 128, encoded as 255
      this exponent is not used for number range
      valid numbers have exp. 127 max because of this infinity / NaN rule
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
    having maximal exponent 128, encoded as 255
      this exponent is not used for number range
      valid numbers have exp. 127 max because of this infinity / NaN rule
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
    http://www.binaryconvert.com/result_float.html?hexadecimal=FFFFFFFF
*/
    input_values[++i] = 0.0/0.0;
    expected_1bit[i] = -1.0/0.0;
    expected_2bit[i] = input_values[i];
    expected_pre2max_bits[i] = input_values[i];
    expected_premax_bits[i] = input_values[i];

    unsigned long bitmask = 0xFFFFFFFF;
    input_values[++i] = *((float*)(&bitmask));
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

float expected_value(char sigbits, size_t i) {
    switch(sigbits) {
        case 1:
            return expected_1bit[i];
        case 2:
            return expected_2bit[i];
        // Remember: sigbits = mantissa bits + 1
        case MANTISSA_LENGTH_FLOAT - 1:
            return expected_pre2max_bits[i];
        case MANTISSA_LENGTH_FLOAT:
            return expected_premax_bits[i];
        case MANTISSA_LENGTH_FLOAT + 1:
            return expected_max_bits[i];
    }
}

int same_value(float expected, float test) {
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
    int ret = scil_context_create(&context, SCIL_TYPE_FLOAT, 0, NULL, &hints);

    if(ret != SCIL_NO_ERR){
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

    for(size_t i = 0; i < count; ++i){
        buffer_in[i] = input_values[i];
    }

    size_t out_size;
    ret = scil_compress(buffer_out, compressed_size, buffer_in, &dims, &out_size, context);
    scil_decompress(SCIL_TYPE_FLOAT, buffer_end, &dims, buffer_out, out_size, buffer_tmp);

    if(ret != SCIL_NO_ERR){
      return -1;
    }

    int errors = 0;
    printf("#Input value,Expected value,Value after comp-decomp,Status\n");
    for (size_t i = 0; i < count; ++i) {
        float expected = expected_value(sigbits, i);
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
    errors += test_sigbits(MANTISSA_LENGTH_FLOAT-1);
    errors += test_sigbits(MANTISSA_LENGTH_FLOAT);
    if(test_sigbits(MANTISSA_LENGTH_FLOAT + 1) != -1){
      errors++;
    }
    return errors;
}
