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
#include <scil.h>
#include <scil-error.h>

#include <assert.h>
#include <stdio.h>

#define SUCCESS 0

/*
 This example demonstrates how the precision of datatypes can be defined.
 */
int main(){
  int ret;

  scil_user_hints_t h;
  scil_user_hints_initialize(& h);

  // hints for the precision
  h.relative_tolerance_percent = 10;
  h.relative_err_finest_abs_tolerance = 0.1; // the finest granularity

  h.absolute_tolerance = 0.5;
  h.significant_digits = 2;
  h.significant_bits = 18;

  // hints for the performance
  // we want to use it for improving network speed, therefore, unit is NETWORK
  h.comp_speed = (scil_performance_hint_t) { .unit = SCIL_PERFORMANCE_NETWORK, .multiplier = 1.5 };

  // for decompression we set a fixed performance
  h.decomp_speed.unit = SCIL_PERFORMANCE_GIB;
  h.decomp_speed.multiplier = 3.5;

  scil_context_t* ctx;
  ret = scil_context_create(&ctx, SCIL_TYPE_DOUBLE, 0, NULL, & h);
  assert(ret == SCIL_NO_ERR);

  // retrieve effectively set hints:
  scil_user_hints_t e = scil_get_effective_hints(ctx);
  scil_user_hints_print(& e);

  // now you can compress/decompress

  ret = scil_destroy_context(ctx);
  assert(ret == SCIL_NO_ERR);

  printf("OK\n");

  return SUCCESS;
}
