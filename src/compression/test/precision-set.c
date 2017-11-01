// This file is part of SCIL.
//
// SCIL is free software: you can redistrbute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SCIL is distrbuted in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with SCIL.  If not, see <http://www.gnu.org/licenses/>.

#include <scil.h>
#include <scil-error.h>
#include <scil-util.h>

#include <assert.h>
#include <stdio.h>

#define SUCCESS 0

static void test(enum SCIL_Datatype type, int digits, int bits, int expected_digits, int expected_bits){
  if(expected_digits == -1 || expected_bits == -1) { return; }

  printf("d: %d b: %d ed: %d eb: %d\n", digits, bits, expected_digits, expected_bits);

  int ret;
  scil_user_hints_t h;
  scil_user_hints_initialize(& h);

  h.significant_digits = digits;
  h.significant_bits = bits;

  scil_context_t* ctx;
  ret = scil_context_create(&ctx, type, 0, NULL, & h);
  assert(ret == SCIL_NO_ERR);
  // retrieve effectively set hints:
  scil_user_hints_t e = scil_get_effective_hints(ctx);

  scil_user_hints_print(& e);

  if (e.significant_bits != SCIL_ACCURACY_INT_FINEST){
    assert(e.significant_digits == expected_digits);
    assert(e.significant_bits == expected_bits);
  }

  ret = scil_destroy_context(ctx);
  assert(ret == SCIL_NO_ERR);
}

/*
 This test checks that the values as precision digits and precision bits set into SCIL are converted properly.
 */
int main(){
  // map between significant bits and digits
  int sig_bits[MANTISSA_MAX_LENGTH_P1]; //for any number of digits
  int sig_digits[MANTISSA_MAX_LENGTH_P1]; // for any number of bits
  // test basics
  for (int i= 0; i < MANTISSA_MAX_LENGTH_P1; i++){
    sig_bits[i] = scilU_convert_significant_decimals_to_bits(i);
    sig_digits[i] = scilU_convert_significant_bits_to_decimals(i);
  }

  // test internal conversion
  test(SCIL_TYPE_DOUBLE, SCIL_ACCURACY_INT_IGNORE, SCIL_ACCURACY_INT_IGNORE, SCIL_ACCURACY_INT_IGNORE, SCIL_ACCURACY_INT_IGNORE);

  for (int d = 0; d < 16; d++){
    for (int b = 0; b < MANTISSA_MAX_LENGTH_P1; b++){
      int e_d = d;
      int e_b = b;
      if (b != 0){
        e_d = sig_digits[b];
      }
      if (d != 0){
        if (b == 0){
          e_b = sig_bits[d];
        }else{
          // take the maximum
          int p_b = sig_bits[d];
          e_b = max(p_b, b);
          int p_d = sig_digits[b];
          e_d = max(p_d, d);
        }
      }

      test(SCIL_TYPE_DOUBLE, d, b, e_d, e_b);
      test(SCIL_TYPE_FLOAT, d, b, (e_d > 6 || e_b > 23 ? -1 : e_d), e_b > 23 ? -1 : e_b);
    }
  }

  printf("OK\n");

  return SUCCESS;
}
