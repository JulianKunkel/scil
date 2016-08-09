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

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <float.h>
#include <math.h>

#include <scil-util.h>
#include <scil-algo-chooser.h>
#include <scil-patterns.h>
#include <scil-internal.h>

#define allocate(type, name, count) type* name = (type*)malloc(count * sizeof(type))

#define DEFAULT_ABSOLUTE_ERROR_TOLERANCE 0.005
#define DEFAULT_SIGNIFICANT_BITS         7

// #############################################################################
// # Data Characteristics Aquisition
// #############################################################################

static double get_data_minimum(const double* const data, size_t count){

    double min = INFINITY;

    for (size_t i = 0; i < count; i++) {
        if (data[i] < min) { min = data[i]; }
    }

    return min;
}

static double get_data_maximum(const double* const data, size_t count){

    double max = -INFINITY;

    for (size_t i = 0; i < count; i++) {
        if (data[i] > max) { max = data[i]; }
    }

    return max;
}

static double get_data_mean(const double* const data, size_t count){

      double mn = 0.0;

      for (size_t i = 0; i < count; i++) {
          mn += data[i];
      }

      return mn / count;
}

static int compdblp(const void* a, const void* b){

    if (*(double*)a > *(double*)b){ return 1; }
    if (*(double*)a < *(double*)b){ return -1; }
    return 0;
}
static double get_data_median(const double* const data, size_t count){

    allocate(double, tmp_buf, count);
    memcpy(tmp_buf, data, count * sizeof(double));

    qsort(tmp_buf, count, sizeof(double), compdblp);

    double median = 0.0;

    if (count % 2 == 0) { median = 0.5 * (tmp_buf[count/2] + tmp_buf[count/2 + 1]); }
    else                { median = tmp_buf[count/2]; }

    free(tmp_buf);

    return median;
}

static double get_data_std_deviation(const double* const data, size_t count, double mean){

    double variance_sum = 0.0;

    for (size_t i = 0; i < count; i++) {
        double dif = data[i] - mean;
        variance_sum += dif * dif;
    }

    return sqrt(variance_sum / count);
}

static int get_data_characteristics(const double* const data, size_t count,
                                    double* const minimum   , double* const maximum,
                                    double* const mean      , double* const median,
                                    double* const std_deviation){

    *minimum       = get_data_minimum(data, count);
    *maximum       = get_data_maximum(data, count);
    *mean          = get_data_mean(data, count);
    *median        = get_data_median(data, count);
    *std_deviation = get_data_std_deviation(data, count, *mean);

    return 0;
}

// #############################################################################
// # Data generation & Benchmark
// #############################################################################

typedef struct {
    char* pattern_name;
    float min_min,  max_max,  step;      // Minimum iteration commences by multiplying step to itervar
    float arg_min,  arg_max,  arg_step;  // Arg1 iteration commences by summing step to itervar
    float arg2_min, arg2_max, arg2_step; // Arg2 iteration commences by summing step to itervar
} pattern_iterator_data_t;

static pattern_iterator_data_t pid;
pid.min_min  = 1e-6; pid.max_max  = 1.1e6; pid.step      = 10.0f;
pid.arg_min  = 0.0f; pid.arg_max  = 10.1f; pid.arg_step  = 1.0f;
pid.arg2_min = 0.0f; pid.arg2_max = 10.1f; pid.arg2_step = 1.0f;

double* data = NULL;
scil_dims dims;

static int benchmark_data(const double* const data, const scil_dims* const dims){

    double minimum, maximum;
    double mean, median;
    double std_deviation;

    get_data_characteristics(data          , scil_get_data_count(dims),
                             &minimum      , &maximum,
                             &mean         , &median,
                             &std_deviation);

    printf("#minimum,maximum,mean,median,standard deviation\n");
    printf("%f,%f,%f,%f,%f\n", minimum, maximum, mean, median, std_deviation);

    return 0;
}

static void iterate_4_algorithms(){

}

static void iterate_3_3_arg2(float min, float max, float arg){

}
static void iterate_3_2_arg(float min, float max){

}
static void iterate_3_1_mn_mx() {

    float min_min = pid.min_min;
    float max_max = pid.max_max;
    float step    = pid.step;

    float big_neg_limit = -0.9e-6;
    float big_pos_limit = 1.1e6;

    // Negative minimum
    for (float min = -max_max; min < big_neg_limit; min /= step) {

        // Negative maximum
        for (float max = min; max < big_neg_limit; max /= step) {
            iterate_3_2_arg(min, max);
        }

        // Zero maximum
        iterate_3_2_arg(min, 0.0f);

        // Positive maximum
        for (float max = min_min; max < big_pos_limit; max *= step) {
            iterate_3_2_arg(min, max);
        }
    }

    // Zero minimum
    { float min = 0.0f;

        // Zero maximum
        iterate_3_2_arg(min, 0.0f);

        // Positive maximum
        for (float max = min_min; max < big_pos_limit; max *= step) {
            iterate_3_2_arg(min, max);
        }
    }

    // Positive minimum
    for (float min = min_min; min < big_pos_limit; min *= step) {

        // Positive maximum
        for (float max = min; max < big_pos_limit; max *= step) {
            iterate_3_2_arg(min, max);
        }
    }
}

static void iterate_2_patterns() {

    pid.name = "constant"; // TODO: for each patter one function
    iterate_3_1_mn_mx();
    pid.name = "random";
    iterate_3_1_mn_mx();
    pid.name = "step";
    iterate_3_1_mn_mx();
    pid.name = "sin";
    iterate_3_1_mn_mx();
    pid.name = "poly4";
    iterate_3_1_mn_mx();
}

static void iterate_1_dimensions(size_t count){

    for (uint8_t d_size = 1; d_size < 5; ++d_size) {

        size_t count_per_dim = (size_t)(pow((double)count, 1.0 / d_size));

        switch (d_size) {
        case 1: scil_init_dims_1d(&dims, count_per_dim); break;
        case 2: scil_init_dims_2d(&dims, count_per_dim, count_per_dim); break;
        case 3: scil_init_dims_3d(&dims, count_per_dim, count_per_dim, count_per_dim); break;
        case 4: scil_init_dims_4d(&dims, count_per_dim, count_per_dim, count_per_dim, count_per_dim); break;
        }

        data = (double*)malloc(scil_get_data_size(SCIL_TYPE_DOUBLE, &dims));

        iterate_2_patterns();

        free(dims);
        free(data);
    }
}

static void iterate_0_datasize(){

    const size_t min_count = 10;
    const size_t max_count = 100000001;

    for (size_t size = min_count; size < max_count; size *= 10) {
        iterate_1_dimensions(size_t count);
    }
}

int main(void){

    benchmark_data(data, &dims);

    return 0;
}
