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

#include <scil-error.h>
#include <scil-util.h>

#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <string.h>

#pragma GCC diagnostic ignored "-Wunused-parameter"

static int constant(scil_dims * dims, double * buffer, float mn, float mx, float arg, float arg2){
  size_t count = scil_get_data_count(dims);
  for (size_t i=0; i < count; i++){
    buffer[i] = mn;
  }
  return SCIL_NO_ERR;
}

static int steps(scil_dims * dims, double * buffer, float mn, float mx, float arg, float arg2){
  if(arg <= 0 || scilU_float_equal(mn, mx) ){
    return SCIL_EINVAL;
  }
  int steps = (int) arg;
  float step_height = (mx - mn) / (steps-1); // Possible division by 0
  size_t count = scil_get_data_count(dims);
  for (size_t i=0; i < count; i++){
    buffer[i] = mn + (i % steps) * step_height;
  }
  return SCIL_NO_ERR;
}

static int steps2(scil_dims * dims, double * buffer, float mn, float mx, float arg, float arg2)
{
    size_t count      = scil_get_data_count(dims);
    int steps_count   = arg;
    int step_width    = count / steps_count;
    float step_height = steps_count <= 1 ? 0 : (mx - mn) / (steps_count - 1);
    for (size_t i = 0; i < count; ++i){
        buffer[i] = mn + i / step_width * step_height;
    }
    return SCIL_NO_ERR;
}

static void rotate2d(float* a, float* b, float* c, float mn, float mx, size_t x_size, size_t y_size)
{
    float a_tmp = -*b * y_size / x_size;
    float b_tmp = *a * x_size / y_size;
    float c_tmp = mn + *b * y_size; //where is *c?

    *a = a_tmp;
    *b = b_tmp;
    *c = c_tmp;
}

static int linear1d(double* buffer, scil_dims* dims, float mn, float mx)
{
    size_t nmemb = scil_get_data_count(dims);
    double r   = random() / RAND_MAX;
    float a    = r > 0.5 ? mn : mx;
    float last = r > 0.5 ? mx : mn;
    float b    = (last - a) / nmemb;
    for (size_t x = 0; x < nmemb; x++) {
        buffer[x] = a + b * x;
    }
    return SCIL_NO_ERR;
}
static int linear2d(double* buffer, scil_dims* dims, float mn, float mx)
{
    size_t x_size = dims->length[0];
    size_t y_size = dims->length[1];
    // a * x_size + b * y_size = mx - mn
    // a < (mx - mn) / x_size
    float r = random() / RAND_MAX;
    float a = r * (mx - mn) / x_size;
    float b = ((mx - mn) - a * x_size) / y_size;
    float c = mn;

    int r2 = random() % 4;
    for (int i = 0; i < r2; ++i)
        rotate2d(&a, &b, &c, mn, mx, x_size, y_size);

    for (size_t y = 0; y < y_size; ++y){
        for (size_t x = 0; x < x_size; ++x){
            size_t i = y*x_size + x;
            buffer[i] = a * x + b * y + c;
        }
    }
    return SCIL_NO_ERR;
}

static int linear3d(double* buffer, scil_dims* dims, float mn, float mx)
{
  return SCIL_NO_ERR;
}

static int linear4d(double* buffer, scil_dims* dims, float mn, float mx)
{
  return SCIL_NO_ERR;
}

static int linear(scil_dims * dims, double * buffer, float mn, float mx, float arg, float arg2)
{
    switch(dims->dims){
    case 1: return linear1d(buffer, dims, mn, mx);
    case 2: return linear2d(buffer, dims, mn, mx);
    case 3: return linear3d(buffer, dims, mn, mx);
    case 4: return linear4d(buffer, dims, mn, mx);
    }
}

static int rnd(scil_dims * dims, double * buffer, float mn, float mx, float arg, float arg2){
  srand((int) arg);
  size_t count = scil_get_data_count(dims);
  double delta = (mx - mn);
  for (size_t i=0; i < count; i++){
    buffer[i] = (double) mn + (random() / ((double) RAND_MAX))*delta;
  }
  return SCIL_NO_ERR;
}

static int p_sin(scil_dims * dims, double * buffer, float mn, float mx, float arg, float arg2){
  const int frequencyCount = (int) arg2;
  double highFrequency = (double)arg;

  if(scilU_float_equal(mn, mx) || frequencyCount <= 0){
    return SCIL_EINVAL;
  }
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

  scilPI_fix_min_max(buffer, dims, mn, mx);

  return SCIL_NO_ERR;
}

typedef struct{
  int points;
  double * values;
} poly4_data;

static void m_poly_func(double * data, scil_dims pos, scil_dims size, int * iter, void * user_ptr){
  poly4_data * usr = (poly4_data*) user_ptr;
  double val = 0;
  for(int d=0; d < pos.dims; d++){
    double new = 1;
    double * v = & usr->values[usr->points * d];
    for(int i=0; i < usr->points; i++){
      new = new * (pos.length[d] - v[i]);
    }
    val += new;
  }

  data[scilG_data_pos(& pos, & size)] = val;
}


static int poly4(scil_dims * dims, double * data, float mn, float mx, float arg, float arg2){
  scil_dims pos;
  scil_copy_dims_array(& pos, *dims);
  memset(pos.length, 0, sizeof(size_t)*pos.dims);

  srand((int) arg);

  poly4_data usr;
  // initialize random 0 points
  usr.points = (int) arg2;
  assert(usr.points > 0);
  usr.points += 2;

  usr.values = (double*) malloc(sizeof(double)*dims->dims * usr.points);

  // initialize values
  double * vals = usr.values;
  for(int d=0; d < dims->dims; d++){
    int frac = dims->length[d] / usr.points;
    for(int i=0; i < usr.points; i++){
      *vals = (rand() % (frac*10)) / 10.0 + i*frac;
      vals++;
    }
    usr.values[d*usr.points] = 0;
    usr.values[(d+1)*usr.points - 1] = dims->length[d];
  }

  scilG_iter(data, *dims, pos, *dims, NULL, & m_poly_func, &usr );
  free(usr.values);

  scilPI_fix_min_max(data, dims, mn, mx);

  return SCIL_NO_ERR;
}


scil_pattern scil_pattern_constant = { &constant, "constant" };
scil_pattern scil_pattern_steps = { &steps , "steps" };
scil_pattern scil_pattern_rnd = { &rnd , "random" };
scil_pattern scil_pattern_sin = { &p_sin , "sin" };
scil_pattern scil_pattern_poly4 = {&poly4, "poly4"};
