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

static int constant(double* buffer, const scil_dims_t* dims, double mn, double mx, double arg, double arg2, int seed){
  size_t count = scil_dims_get_count(dims);
  for (size_t i=0; i < count; i++){
    buffer[i] = mn;
  }
  return SCIL_NO_ERR;
}

static int steps1d(double* buffer, const scil_dims_t* dims, double mn, double mx, int step_size)
{
    double gradient = (mx - mn) / (step_size-1);

    size_t x_size = dims->length[0];

    for (size_t x = 0; x < x_size; x++){
        buffer[x] = mn + gradient * (x % step_size);
    }

    return SCIL_NO_ERR;
}

static int steps2d(double* buffer, const scil_dims_t* dims, double mn, double mx, int step_size)
{
    double gradient = 0.5 * (mx - mn) / (step_size - 1);

    size_t x_size = dims->length[0];
    size_t y_size = dims->length[1];

    for (size_t y = 0; y < y_size; ++y){
        for (size_t x = 0; x < x_size; ++x){
            buffer[x_size * y + x] = mn + gradient * ((x + y) % (2 * step_size));
        }
    }

    return SCIL_NO_ERR;
}

static int steps3d(double* buffer, const scil_dims_t* dims, double mn, double mx, int step_size)
{
    double gradient = 0.3333 * (mx - mn) / (step_size - 1);

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

static int steps4d(double* buffer, const scil_dims_t* dims, double mn, double mx, int step_size)
{
    double gradient = 0.25 * (mx - mn) / (step_size - 1);

    size_t x_size = dims->length[0];
    size_t y_size = dims->length[1];
    size_t z_size = dims->length[2];
    size_t w_size = dims->length[3];

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

static int steps(double* buffer, const scil_dims_t* dims, double mn, double mx, double arg, double arg2, int seed)
{
    int corrected_arg = arg < 2.0 ? 2 : (int)arg;

    //if (scilU_double_equal(mn, mx))
    //    return SCIL_EINVAL;

    switch (dims->dims) {
        case 1: steps1d(buffer, dims, mn, mx, corrected_arg); break;
        case 2: steps2d(buffer, dims, mn, mx, corrected_arg); break;
        case 3: steps3d(buffer, dims, mn, mx, corrected_arg); break;
        case 4: steps4d(buffer, dims, mn, mx, corrected_arg); break;
    }

    return SCIL_NO_ERR;
}

static void rotate2d(double* a, double* b, double* c, double mn, double mx, size_t x_size, size_t y_size)
{
    double a_tmp = -*b * y_size / x_size;
    double b_tmp = *a * x_size / y_size;
    double c_tmp = mn + *b * y_size; //where is *c?

    *a = a_tmp;
    *b = b_tmp;
    *c = c_tmp;
}

static int linear1d(double* buffer, const scil_dims_t* dims, double mn, double mx)
{
    size_t nmemb = scil_dims_get_count(dims);
    double r   = random() / RAND_MAX;
    double a    = r > 0.5 ? mn : mx;
    double last = r > 0.5 ? mx : mn;
    double b    = (last - a) / nmemb;
    for (size_t x = 0; x < nmemb; x++) {
        buffer[x] = a + b * x;
    }
    return SCIL_NO_ERR;
}
static int linear2d(double* buffer, const scil_dims_t* dims, double mn, double mx)
{
    size_t x_size = dims->length[0];
    size_t y_size = dims->length[1];
    // a * x_size + b * y_size = mx - mn
    // a < (mx - mn) / x_size
    double r = random() / RAND_MAX;
    double a = r * (mx - mn) / x_size;
    double b = ((mx - mn) - a * x_size) / y_size;
    double c = mn;

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

static int linear3d(double* buffer, const scil_dims_t* dims, double mn, double mx)
{
  return SCIL_NO_ERR;
}

static int linear4d(double* buffer, const scil_dims_t* dims, double mn, double mx)
{
  return SCIL_NO_ERR;
}

static int linear(double* buffer, const scil_dims_t* dims, double mn, double mx, double arg, double arg2, int seed)
{
    switch(dims->dims){
    case 1: return linear1d(buffer, dims, mn, mx);
    case 2: return linear2d(buffer, dims, mn, mx);
    case 3: return linear3d(buffer, dims, mn, mx);
    case 4: return linear4d(buffer, dims, mn, mx);
    default: return SCIL_UNKNOWN_ERR;
    }
}

static int rnd(double* buffer, const scil_dims_t* dims, double mn, double mx, double arg, double arg2, int seed){
  srand(seed);
  size_t count = scil_dims_get_count(dims);
  double delta = (mx - mn);
  for (size_t i=0; i < count; i++){
    buffer[i] = (double) mn + (random() / ((double) RAND_MAX))*delta;
  }
  return SCIL_NO_ERR;
}

static int p_sin1d(double* buffer, const scil_dims_t* dims, double base_freq, int freq_count)
{
    const size_t x_size = dims->length[0];

    const double x_scale_b = 2 * M_PI * base_freq / x_size;
    const double falloff_b = 1.0f;

    double x_scale = x_scale_b;

    for (size_t x = 0; x < x_size; x++)
    {
        double value = 0.0f;
        double falloff = 1.0f;
        for(int freq = 1; freq <= freq_count; ++freq)
        {
            value += sin(x * x_scale) * falloff;
            x_scale *= 2;
            falloff /= 2;
        }
        buffer[x] = value;
        x_scale = x_scale_b;
        falloff = falloff_b;
    }

    return SCIL_NO_ERR;
}

static int p_sin2d(double* buffer, const scil_dims_t* dims, double base_freq, int freq_count)
{
    const size_t x_size = dims->length[0];
    const size_t y_size = dims->length[1];

    const double scale = 2 * M_PI * base_freq;
    const double x_scale_b = scale / x_size;
    const double y_scale_b = scale / y_size;
    const double falloff_b = 1.0f;

    double x_scale = x_scale_b;
    double y_scale = y_scale_b;
    double falloff = falloff_b;

    for (size_t y = 0; y < y_size; y++){
        for (size_t x = 0; x < x_size; x++)
        {
            double value = 0.0f;
            for(int freq = 1; freq <= freq_count; ++freq){
                value += (sin(x * x_scale) + sin(y * y_scale)) * falloff;
                x_scale *= 2;
                y_scale *= 2;
                falloff /= 2;
            }
            buffer[y * x_size + x] = value;

            x_scale = x_scale_b;
            y_scale = y_scale_b;
            falloff = falloff_b;
        }
    }

    return SCIL_NO_ERR;
}

static int p_sin3d(double* buffer, const scil_dims_t* dims, double base_freq, int freq_count)
{
    const size_t x_size = dims->length[0];
    const size_t y_size = dims->length[1];
    const size_t z_size = dims->length[2];

    const double scale = 2 * M_PI * base_freq;
    const double x_scale_b = scale / x_size;
    const double y_scale_b = scale / y_size;
    const double z_scale_b = scale / z_size;
    const double falloff_b = 1.0f;

    double x_scale = x_scale_b;
    double y_scale = y_scale_b;
    double z_scale = z_scale_b;
    double falloff = falloff_b;

    for (size_t z = 0; z < z_size; z++){
        for (size_t y = 0; y < y_size; y++){
            for (size_t x = 0; x < x_size; x++)
            {
                double value = 0.0f;
                for(int freq = 1; freq <= freq_count; ++freq){
                    value += (sin(x * x_scale) + sin(y * y_scale) + sin(z * z_scale)) * falloff;
                    x_scale *= 2;
                    y_scale *= 2;
                    z_scale *= 2;
                    falloff /= 2;
                }
                buffer[((z * y_size) + y) * x_size + x] = value;

                x_scale = x_scale_b;
                y_scale = y_scale_b;
                z_scale = z_scale_b;
                falloff = falloff_b;
            }
        }
    }

    return SCIL_NO_ERR;
}

static int p_sin4d(double* buffer, const scil_dims_t* dims, double base_freq, int freq_count)
{
    const size_t x_size = dims->length[0];
    const size_t y_size = dims->length[1];
    const size_t z_size = dims->length[2];
    const size_t w_size = dims->length[3];

    const double scale = 2 * M_PI * base_freq;
    const double x_scale_b = scale / x_size;
    const double y_scale_b = scale / y_size;
    const double z_scale_b = scale / z_size;
    const double w_scale_b = scale / w_size;
    const double falloff_b = 1.0f;

    double x_scale = x_scale_b;
    double y_scale = y_scale_b;
    double z_scale = z_scale_b;
    double w_scale = w_scale_b;
    double falloff = falloff_b;

    for (size_t w = 0; w < w_size; w++){
        for (size_t z = 0; z < z_size; z++){
            for (size_t y = 0; y < y_size; y++){
                for (size_t x = 0; x < x_size; x++)
                {
                    double value = 0.0f;
                    for(int freq = 1; freq <= freq_count; ++freq){
                        value += (sin(x * x_scale) + sin(y * y_scale) + sin(z * z_scale) + sin(w * w_scale)) * falloff;
                        x_scale *= 2;
                        y_scale *= 2;
                        z_scale *= 2;
                        w_scale *= 2;
                        falloff /= 2;
                    }
                    buffer[((w * z_size + z) * y_size + y) * x_size + x] = value;

                    x_scale = x_scale_b;
                    y_scale = y_scale_b;
                    z_scale = z_scale_b;
                    w_scale = w_scale_b;
                    falloff = falloff_b;
                }
            }
        }
    }

    return SCIL_NO_ERR;
}

static int p_sin(double* buffer, const scil_dims_t* dims, double mn, double mx, double arg, double arg2, int seed)
{
  double base_freq = (double)arg;
  const int freq_count = (int) arg2;

  if(scilU_double_equal(mn, mx) || freq_count <= 0){
    return SCIL_EINVAL;
  }

  int ret = SCIL_UNKNOWN_ERR;

  switch (dims->dims) {
      case 1: ret = p_sin1d(buffer, dims, base_freq, freq_count); break;
      case 2: ret = p_sin2d(buffer, dims, base_freq, freq_count); break;
      case 3: ret = p_sin3d(buffer, dims, base_freq, freq_count); break;
      case 4: ret = p_sin4d(buffer, dims, base_freq, freq_count); break;
  }

  scilP_change_data_scale(buffer, dims, mn, mx);

  return ret;
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

    data[scilU_data_pos(pos, size)] = val;
}


static int poly4(double* data, const scil_dims_t* dims, double mn, double mx, double arg, double arg2, int seed){
  scil_dims_t pos;
  scil_dims_copy(&pos, dims);
  memset(pos.length, 0, sizeof(size_t) * pos.dims);

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
      *vals = (rand() % (1+frac*10)) / 10.0 + i*frac;
      vals++;
    }
    usr.values[d*usr.points] = 0;
    usr.values[(d+1)*usr.points - 1] = dims->length[d];
  }

  scilU_iter(data, dims, &pos, dims, NULL, & m_poly_func, &usr );
  free(usr.values);

  scilP_change_data_scale(data, dims, mn, mx);

  return SCIL_NO_ERR;
}


scil_pattern_t scil_pattern_constant = { &constant, "constant" };
scil_pattern_t scil_pattern_steps = { &steps , "steps" };
scil_pattern_t scil_pattern_rnd = { &rnd , "random" };
scil_pattern_t scil_pattern_sin = { &p_sin , "sin" };
scil_pattern_t scil_pattern_linear = { &linear , "linear" };
scil_pattern_t scil_pattern_poly4 = {&poly4, "poly4"};
