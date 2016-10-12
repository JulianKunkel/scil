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

static int constant(double* buffer, const scil_dims_t* dims, float mn, float mx, float arg, float arg2){
  size_t count = scilPr_get_dims_count(dims);
  for (size_t i=0; i < count; i++){
    buffer[i] = mn;
  }
  return SCIL_NO_ERR;
}

static int steps1d(double* buffer, const scil_dims_t* dims, float mn, float mx, int step_size)
{
    float gradient = (mx - mn) / (step_size-1);

    size_t x_size = dims->length[0];

    for (size_t x = 0; x < x_size; x++){
        buffer[x] = mn + gradient * (x % step_size);
    }

    return SCIL_NO_ERR;
}

static int steps2d(double* buffer, const scil_dims_t* dims, float mn, float mx, int step_size)
{
    float gradient = 0.5f * (mx - mn) / (step_size - 1);

    size_t x_size = dims->length[0];
    size_t y_size = dims->length[1];

    for (size_t y = 0; y < y_size; ++y){
        for (size_t x = 0; x < x_size; ++x){
            buffer[x_size * y + x] = mn + gradient * (x + y) % (2 * step_size);
        }
    }

    return SCIL_NO_ERR;
}

static int steps3d(double* buffer, const scil_dims_t* dims, float mn, float mx, int step_size)
{
    float gradient = 0.3333f * (mx - mn) / (step_size - 1);

    size_t x_size = dims->length[0];
    size_t y_size = dims->length[1];
    size_t z_size = dims->length[2];

    for (size_t z = 0; z < z_size; ++z){
        for (size_t y = 0; y < y_size; ++y){
            for (size_t x = 0; x < x_size; ++x){
                buffer[(z * y_size + y) * x_size + x]
                    = mn + gradient * ((x + y + z) % (3 * step_size));
            }
        }
    }

    return SCIL_NO_ERR;
}

static int steps4d(double* buffer, const scil_dims_t* dims, float mn, float mx, int step_size)
{
    float gradient = 0.25f * (mx - mn) / (step_size - 1);

    size_t x_size = dims->length[0];
    size_t y_size = dims->length[1];
    size_t z_size = dims->length[2];
    size_t w_size = dims->length[2];

    for (size_t w = 0; w < w_size; ++w){
        for (size_t z = 0; z < z_size; ++z){
            for (size_t y = 0; y < y_size; ++y){
                for (size_t x = 0; x < x_size; ++x){
                    buffer[((w * z_size + z) * y_size + y) * x_size + x]
                        = mn + gradient * ((x + y + z + w) % ( 4 * step_size));
                }
            }
        }
    }

    return SCIL_NO_ERR;
}

static int steps(double* buffer, const scil_dims_t* dims, float mn, float mx, float arg, float arg2)
{
    if (arg < 1.5f || scilU_float_equal(mn, mx))
        return SCIL_EINVAL;

    switch (dims->dims) {
        case 1: steps1d(buffer, dims, mn, mx, (int) arg); break;
        case 2: steps2d(buffer, dims, mn, mx, (int) arg); break;
        case 3: steps3d(buffer, dims, mn, mx, (int) arg); break;
        case 4: steps4d(buffer, dims, mn, mx, (int) arg); break;
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

static int linear1d(double* buffer, const scil_dims_t* dims, float mn, float mx)
{
    size_t nmemb = scilPr_get_dims_count(dims);
    double r   = random() / RAND_MAX;
    float a    = r > 0.5 ? mn : mx;
    float last = r > 0.5 ? mx : mn;
    float b    = (last - a) / nmemb;
    for (size_t x = 0; x < nmemb; x++) {
        buffer[x] = a + b * x;
    }
    return SCIL_NO_ERR;
}
static int linear2d(double* buffer, const scil_dims_t* dims, float mn, float mx)
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

static int linear3d(double* buffer, const scil_dims_t* dims, float mn, float mx)
{
  return SCIL_NO_ERR;
}

static int linear4d(double* buffer, const scil_dims_t* dims, float mn, float mx)
{
  return SCIL_NO_ERR;
}

static int linear(double* buffer, const scil_dims_t* dims, float mn, float mx, float arg, float arg2)
{
    switch(dims->dims){
    case 1: return linear1d(buffer, dims, mn, mx);
    case 2: return linear2d(buffer, dims, mn, mx);
    case 3: return linear3d(buffer, dims, mn, mx);
    case 4: return linear4d(buffer, dims, mn, mx);
    }
}

static int rnd(double* buffer, const scil_dims_t* dims, float mn, float mx, float arg, float arg2){
  srand((int) arg);
  size_t count = scilPr_get_dims_count(dims);
  double delta = (mx - mn);
  for (size_t i=0; i < count; i++){
    buffer[i] = (double) mn + (random() / ((double) RAND_MAX))*delta;
  }
  return SCIL_NO_ERR;
}

static int p_sin(double* buffer, const scil_dims_t* dims, float mn, float mx, float arg, float arg2){
  const int frequencyCount = (int) arg2;
  double highFrequency = (double)arg;

  if(scilU_float_equal(mn, mx) || frequencyCount <= 0){
    return SCIL_EINVAL;
  }
  int64_t max_potenz = 1<<frequencyCount;
  const double pi = highFrequency*M_PI*2;
  const size_t* len = dims->length;

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

  scilPI_change_data_scale(buffer, dims, mn, mx);

  return SCIL_NO_ERR;
}

typedef struct{
  int points;
  double * values;
} poly4_data;

static void m_poly_func(double* data,
                        const scil_dims_t* pos,
                        const scil_dims_t* size,
                        int* iter,
                        const void* user_ptr)
{
  poly4_data * usr = (poly4_data*) user_ptr;
  double val = 0;
  for(int d=0; d < pos->dims; d++){
    double new = 1;
    double * v = & usr->values[usr->points * d];
    for(int i=0; i < usr->points; i++){
      new = new * (pos->length[d] - v[i]);
    }
    val += new;
  }

  data[scilG_data_pos(pos, size)] = val;
}


static int poly4(double* data, const scil_dims_t* dims, float mn, float mx, float arg, float arg2){
  scil_dims_t pos;
  scilPr_copy_dims(&pos, dims);
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

  scilG_iter(data, dims, &pos, dims, NULL, & m_poly_func, &usr );
  free(usr.values);

  scilPI_change_data_scale(data, dims, mn, mx);

  return SCIL_NO_ERR;
}


scil_pattern_t scil_pattern_constant = { &constant, "constant" };
scil_pattern_t scil_pattern_steps = { &steps , "steps" };
scil_pattern_t scil_pattern_rnd = { &rnd , "random" };
scil_pattern_t scil_pattern_sin = { &p_sin , "sin" };
scil_pattern_t scil_pattern_poly4 = {&poly4, "poly4"};
