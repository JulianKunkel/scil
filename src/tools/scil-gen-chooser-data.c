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

    *minimum               = get_data_minimum(data, count);
    *maximum               = get_data_maximum(data, count);
    *mean                  = get_data_mean(data, count);
    *median                = get_data_median(data, count);
    *std_deviation         = get_data_std_deviation(data, count, *mean);

    return 0;
}

// #############################################################################
// # Benchmark
// #############################################################################

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



static void iterate_random_pattern_minmax_negmin(scil_dims* dims, double* const data) {

    float min_neg = -1e6;
    float max_neg = -0.99e-6;
    float min_pos = 1e-6;
    float max_pos = 1.01e6;

    for (float min = min_neg; min < max_neg; min *= 0.1f) {

        // Negative maximum
        for (float max = min; max < max_neg; max *= 0.1f){
            scilP_create_pattern_double(dims, data, "random", min, max, 0.0f, 0.0f);
        }

        // Zero maximum
        float max = 0.0f;
        scilP_create_pattern_double(dims, data, "random", min, max, 0.0f, 0.0f);

        // Positive maximum
        for (float max = min_pos; max < max_pos; max *= 10.0f) {
            scilP_create_pattern_double(dims, data, "random", min, max, 0.0f, 0.0f);
        }
    }
}
static void iterate_random_pattern_minmax_zeromin(scil_dims* dims, double* const data) {

    float min_pos = 1e-6;
    float max_pos = 1.01e6;

    float min = 0.0f;

    // Zero maximum
    float max = 0.0f;
    scilP_create_pattern_double(dims, data, "random", min, max, 0.0f, 0.0f);

    // Positive maximum
    for (float max = min_pos; max < max_pos; max *= 10.0f) {
        scilP_create_pattern_double(dims, data, "random", min, max, 0.0f, 0.0f);
    }
}
static void iterate_random_pattern_minmax_posmin(scil_dims* dims, double* const data){

    float min_pos = 1e-6;
    float max_pos = 1.01e6;

    for (float min = min_pos; min < max_pos; min *= 10.0f) {

        // Positive maximum
        for (float max = min; max < max_pos; max *= 10.0f) {
            scilP_create_pattern_double(dims, data, "random", min, max, 0.0f, 0.0f);
        }
    }
}
static void iterate_random_pattern_minmax(scil_dims* dims, double* const data) {

    iterate_random_pattern_minmax_negmin(dims, data);
    iterate_random_pattern_minmax_zeromin(dims, data);
    iterate_random_pattern_minmax_posmin(dims, data);
}

static void iterate_patterns(double* const data, size_t count) {
    for (uint8_t pattern = 0; pattern < 4; ++pattern) {

        char* name = scilP_library_pattern_name(pattern);

        switch (pattern) {
        case 0: //
            float min_min = -1e6;
            float min_max = -0.99e-6;

            //Negative
            for (float min = min_min; min < min_max; min *= 0.1f) {
                for (float max = min * 0.1f; max < )
            }
        }
        scilP_create_pattern_double(&dims, data, name, float mn, float mx, float arg, float arg2)

    }
}

static void iterate_dimensions(){
    for (uint8_t d_size = 1; d_size < 5; ++d_size) {

        scil_dims dims;
        size_t count_per_dim = (size_t)(pow((double)count, 1.0 / d_size));

        switch (d_size) {
        case 1: scil_init_dims_1d(&dims, count_per_dim); break;
        case 2: scil_init_dims_2d(&dims, count_per_dim, count_per_dim); break;
        case 3: scil_init_dims_3d(&dims, count_per_dim, count_per_dim, count_per_dim); break;
        case 4: scil_init_dims_4d(&dims, count_per_dim, count_per_dim, count_per_dim, count_per_dim); break;
        }

        size_t count = (size_t)(pow(count_per_dim, d_size));
        allocate(double, data, count);

        iterate_patterns();

        free(dims);
        free(data);
    }
}

int main(void){

    const size_t min_count = 10;
    const size_t max_count = 100000000;

    for (size_t size = min_count; size < max_count + 1; size *= 10) {
        iterate_dimensions();
    }

    scilP_create_pattern_double(&dims, data, "random", 0.0f, 1.0f, 0.0f, 0.0f);

    for (size_t i = 0; i < 5; i++) {
        printf("%f\n", data[i]);
    }
    printf("[...]\n");
    for (size_t i = count-5; i < count; i++) {
        printf("%f\n", data[i]);
    }

    benchmark_data(data, &dims);

    return 0;
}
