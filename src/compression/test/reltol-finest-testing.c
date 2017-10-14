#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#include <scil-util.h>

double calc_adjusted_finest(double finest, uint8_t mantissa_bit_count) {
  datatype_cast_double cur;
  cur.f = finest;
  //uint8_t shifts = MANTISSA_LENGTH_DOUBLE - mantissa_bit_count;
  //cur.p.mantissa = (cur.p.mantissa >> shifts) << shifts;
  cur.p.mantissa = 0;
  return cur.f;
}

void test_reltol(double value, double reltol, double finest){
  uint8_t mantissa_bits = scilU_relative_tolerance_to_significant_bits(reltol) - 1;

  double use_finest = calc_adjusted_finest(finest, mantissa_bits);

  datatype_cast_double cur;
  cur.f = value;

  printf("%.4f, %.4f, %.4f, %.4f, ", value, reltol, finest, use_finest);

  double result = value;
  if (result < 0) {
    result = -result;
  }
  char* exp_type = "Normal";
  if (result < use_finest) {
    if (result < use_finest / 2.0) {
      result = 0.0;
      printf("Zero,   %.4f\n", result);
    } else {
      result = (cur.p.sign ? -1 : 1) * use_finest;
      printf("Normal, %.4f\n", result);
    }
  } else {
      printf("Normal, as usual\n", result);
  }
}

int main(){
  printf("value, reltol, finest, use_finest, exponent_type, result\n");
  test_reltol(0, 50.0, 0.1);
  test_reltol(0.01, 50.0, 0.1);
  test_reltol(0.04, 50.0, 0.1);
  test_reltol(0.05, 50.0, 0.1);
  test_reltol(0.1, 50.0, 0.1);
  test_reltol(0.4, 50.0, 0.1);
  test_reltol(1, 50.0, 0.1);
  test_reltol(-0.0, 50.0, 0.1);
  test_reltol(-0.01, 50.0, 0.1);
  test_reltol(-0.04, 50.0, 0.1);
  test_reltol(-0.05, 50.0, 0.1);
  test_reltol(-0.1, 50.0, 0.1);
  test_reltol(-0.4, 50.0, 0.1);
  test_reltol(-1, 50.0, 0.1);
  return 0;
}
