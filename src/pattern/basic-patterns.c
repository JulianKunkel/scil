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
#include <scil-util.h>

#include <stdio.h>
#include <math.h>
#include <assert.h>

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
  if(arg <= 0 || scilU_float_equal(mn, mx) ){
    return SCIL_EINVAL;
  }
  int steps = (int) arg;
  float step_width = (mx - mn) / (steps-1);
  size_t count = scil_get_data_count(dims);
  for (size_t i=0; i < count; i++){
    buffer[i] = mn + (i % steps) * step_width;
  }
  return SCIL_NO_ERR;
}

static int rnd(scil_dims * dims, double * buffer, float mn, float mx, float arg){
  if(scilU_float_equal(mn, mx)){
    return SCIL_EINVAL;
  }
  size_t count = scil_get_data_count(dims);
  double delta = (mx - mn);
  for (size_t i=0; i < count; i++){
    buffer[i] = (double) mn + (random() / ((double) RAND_MAX))*delta;
  }
  return SCIL_NO_ERR;
}

static int p_sin(scil_dims * dims, double * buffer, float mn, float mx, float arg){
  const int frequencyCount = (int) arg % 10;

  if(scilU_float_equal(mn, mx) || frequencyCount <= 0){
    return SCIL_EINVAL;
  }
  double highFrequency = ((int) arg / 10) / 10.0;
  int64_t max_potenz = 1<<frequencyCount;
  const double pi = highFrequency*M_PI*2;
  size_t * len = dims->length;

  switch(dims->dims){
    case (1):{
      for (size_t x=0; x < dims->length[0]; x++){
        double var = 0;
        int64_t potenz = max_potenz;
        int64_t divisor = 1;
        for(int f = 1 ; f <= frequencyCount; f++){
          var += potenz * sin((x+1)*f*pi / len[0]);
          potenz /= 2;
          divisor *= 2;
        }
        buffer[x] = var;
      }
      break;
    }case (2):{
      for (size_t y=0; y < dims->length[1]; y++){
        for (size_t x=0; x < dims->length[0]; x++){
          double var = 0;
          int64_t potenz = max_potenz;
          int64_t divisor = 1;
          for(int f = 1 ; f <= frequencyCount; f++){
            var += potenz * sin((x+1)*f*pi/len[0]) + potenz * sin((y+1)*f*pi/len[1]);
            potenz /= 2;
            divisor *= 2;
          }
          buffer[x+y*dims->length[0]] = var;
        }
      }
      break;
    }case (3):{
      for (size_t z=0; z < dims->length[2]; z++){
        for (size_t y=0; y < dims->length[1]; y++){
          for (size_t x=0; x < dims->length[0]; x++){
            double var = 0;
            int64_t potenz = max_potenz;
            int64_t divisor = 1;
            for(int f = 1 ; f <= frequencyCount; f++){
              var += potenz * sin((x+1)*f*pi/len[0]) + potenz * sin((y+1)*f*pi/len[1]) + potenz * sin((z+1)*f*pi/len[2]);
              potenz /= 2;
              divisor *= 2;
            }
            buffer[x+y*len[0]+z*(len[0]*len[1])] = var;
          }
        }
      }      break;
    }case (4):{
      for (size_t w=0; w < dims->length[3]; w++){
        for (size_t z=0; z < dims->length[2]; z++){
          for (size_t y=0; y < dims->length[1]; y++){
            for (size_t x=0; x < dims->length[0]; x++){
              double var = 0;
              int64_t potenz = max_potenz;
              int64_t divisor = 1;
              for(int f = 1 ; f <= frequencyCount; f++){
                var += potenz * sin((x+1)*f*pi/len[0]) + potenz * sin((y+1)*f*pi/len[1]) + potenz * sin((z+1)*f*pi/len[2] + potenz * sin((w+1)*f*pi/len[3]));
                potenz /= 2;
                divisor *= 2;
              }
              buffer[x+y*len[0]+z*(len[0]*len[1])+w*(len[0]*len[1]*len[2])] = var;
            }
          }
        }
      }
      break;
    }default:
      assert(0);
  }

  // fix min + max, first identify min/max
  size_t count = scil_get_data_count(dims);
  double mn_o = 1e308, mx_o=-1e308;
  for (size_t i=0; i < count; i++){
    mn_o = min(mn_o, buffer[i]);
    mx_o = max(mx_o, buffer[i]);
  }

  double scaling = (double)(mx - mn) / (mx_o-mn_o); // intended min/max
  // rescale
  for (size_t i=0; i < count; i++){
    buffer[i] = (double) mn + (buffer[i]-mn_o) *scaling;
  }

  return SCIL_NO_ERR;
}

scil_pattern scil_pattern_constant = { &constant, "constant" };
scil_pattern scil_pattern_steps = { &steps , "steps" };
scil_pattern scil_pattern_rnd = { &rnd , "random" };
scil_pattern scil_pattern_sin = { &p_sin , "sin" };
