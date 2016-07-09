// This file is part of SCIL.
//
// SCIL is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SCIL is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with SCIL.  If not, see <http://www.gnu.org/licenses/>.

#include <assert.h>
#include <stdio.h>

#include <scil-util.h>

#define SUCCESS 0

/*
 This test checks that the values as precision digits and precision bits are converted properly.
 */
int main(){
  // map between significant bits and digits
  int sig_bits[MANTISSA_MAX_LENGTH_P1]; //for any number of digits
  int sig_digits[MANTISSA_MAX_LENGTH_P1]; // for any number of bits
  // test basics
  for (int i= 0; i < MANTISSA_MAX_LENGTH_P1; i++){
    sig_digits[i] = scilU_convert_significant_decimals_to_bits(i);
    sig_bits[i] = scilU_convert_significant_bits_to_decimals(i);
  }
  printf("Digits\n");
  long long unsigned val = 1;
  for (int i= 1; i <= 16; i++){
    printf("%d = %lld: %d bits \n", i, val, sig_digits[i]);
    val *= 10;
  }
  printf("\nBits\n");
  val = 2;

  // compare the output of both conversions
  for (int i= 1; i < MANTISSA_MAX_LENGTH_P1; i++){
    printf("%d = %lld: %d digits \n", i, val -1, sig_bits[i]);
    assert( sig_digits[sig_bits[i]] >= i);
    val *= 2;
  }

  printf("OK\n");

  return SUCCESS;
}
