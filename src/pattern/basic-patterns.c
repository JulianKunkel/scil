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

#include <basic-patterns.h>

#include <stdio.h>

#pragma GCC diagnostic ignored "-Wunused-parameter"

static void m_interpolator(scil_dims * dims, double * buffer){
  size_t count = scil_get_data_count(dims);
  // for now interpolate only in 1D
  double val = buffer[0];
  for (size_t i=1; i < count-1; i++){
    double new = buffer[i];
    buffer[i] = (val + buffer[i+1])/2;
    val = new;
  }
}
scilP_mutator scilP_interpolator = & m_interpolator;

static void m_repeater(scil_dims * dims, double * buffer){
  size_t count = scil_get_data_count(dims) - 1;
  // repeat a value two times, odd become even
  double val = buffer[0];
  for (size_t i=0; i < count; i+= 2){
    buffer[i] = buffer[i+1];
  }
}
scilP_mutator scilP_repeater = & m_repeater;

static int constant(scil_dims * dims, double * buffer, float mn, float mx, float arg){
  size_t count = scil_get_data_count(dims);
  for (size_t i=0; i < count; i++){
    buffer[i] = mn;
  }
  return SCIL_NO_ERR;
}

static int steps(scil_dims * dims, double * buffer, float mn, float mx, float arg){
  if(arg <= 0 || mn == mx){
    return SCIL_EINVAL;
  }
  int steps = (mx - mn);
  float step_width = steps / arg;
  size_t count = scil_get_data_count(dims);
  for (size_t i=0; i < count; i++){
    buffer[i] = mn + (i % steps) * step_width;
  }
  return SCIL_NO_ERR;
}

static int rnd(scil_dims * dims, double * buffer, float mn, float mx, float arg){
  if(mn == mx){
    return SCIL_EINVAL;
  }
  size_t count = scil_get_data_count(dims);
  double delta = (mx - mn);
  for (size_t i=0; i < count; i++){
    buffer[i] = (double) mn + (random() / ((double) RAND_MAX))*delta;
  }
  return SCIL_NO_ERR;
}

scil_pattern scil_pattern_constant = { &constant, "constant" };
scil_pattern scil_pattern_steps = { &steps , "steps" };
scil_pattern scil_pattern_rnd = { &rnd , "random" };
