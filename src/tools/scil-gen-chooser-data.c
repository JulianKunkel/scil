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

#include <scil-algos.h>
#include <scil-algo-chooser.h>
#include <scil-internal.h>
#include <scil-patterns.h>
#include <scil-util.h>

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

float negative_minimum = -1e6;
float negative_limit   = 0.9e-6;
float positive_minimum = 1e-6;
float positive_limit   = 1.1e6;
float factor_step      = 10.0f;

double* data = NULL;
scil_dims dims;

scilPr_user_hints_t hints;
//scilPr_initialize_user_hints(&hints);

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

/* Systematic params
static void iterate_6_poly4_arg2(float mn, float mx, float arg){

    for (int arg2 = 1; arg2 < 33; arg2 *= 2){

        scilP_create_pattern_double(&dims, data, "sin", mn, mx, arg, arg2);
    }
}
static void iterate_5_poly4_arg(float mn, float mx){

    for (int arg = 1; arg < 101; arg *= 10) {

        iterate_6_poly4_arg2(float mn, float mx, float arg)
    }
}
static void iterate_4_poly4_mx_negative(float mn){

    for (float mx = min; mx < negative_limit; mx /= factor_step) {
        iterate_5_poly4_arg(mn, mx);
    }
}
static void iterate_4_poly4_mx_zero(float mn){

    iterate_5_poly4_arg(mn, 0.0f);
}
static void iterate_4_poly4_mx_positive(float mn){

    for (float mx = positive_minimum; mx < positive_limit; mx *= factor_step) {
        iterate_5_poly4_arg(mn, mx);
    }
}
static void iterate_3_poly4_mn(){

    // Negative minimum
    for (float mn = negative_minimum; mn < negative_limit; mn /= factor_step) {

        iterate_4_poly4_mx_negative(mn);
        iterate_4_poly4_mx_zero(mn);
        iterate_4_poly4_mx_positive(mn);
    }

    // Zero minimum
    { float mn = 0.0f;

        iterate_4_poly4_mx_zero(mn);
        iterate_4_poly4_mx_positive(mn);
    }

    // Positive minimum
    for (float mn = positive_minimum; mn < positive_limit; mn *= factor_step) {

        iterate_4_poly4_mx_positive(mn);
    }
}

static void iterate_5_sin_arg(){

    for (int arg = 1; arg < arg_maximum; ++arg) {

        scilP_create_pattern_double(&dims, data, "sin", mn, mx, arg, 0.0f);
    }
}
static void iterate_4_sin_mx_negative(float mn){

    for (float mx = min; mx < negative_limit; mx /= factor_step) {
        iterate_5_sin_arg(mn, mx);
    }
}
static void iterate_4_sin_mx_zero(float mn){

    iterate_5_sin_arg(mn, 0.0f);
}
static void iterate_4_sin_mx_positive(float mn){

    for (float mx = positive_minimum; mx < positive_limit; mx *= factor_step) {
        iterate_5_sin_arg(mn, mx);
    }
}
static void iterate_3_sin_mn(){

    // Negative minimum
    for (float mn = negative_minimum; mn < negative_limit; mn /= factor_step) {

        iterate_4_sin_mx_negative(mn);
        iterate_4_sin_mx_zero(mn);
        iterate_4_sin_mx_positive(mn);
    }

    // Zero minimum
    { float mn = 0.0f;

        iterate_4_sin_mx_zero(mn);
        iterate_4_sin_mx_positive(mn);
    }

    // Positive minimum
    for (float mn = positive_minimum; mn < positive_limit; mn *= factor_step) {

        iterate_4_sin_mx_positive(mn);
    }
}

static void iterate_5_step_arg(float mn, float mx) {

    for (int arg = 1; arg < 65; arg *= 2) {

        scilP_create_pattern_double(&dims, data, "step", mn, mx, arg, 0.0f);
    }
}
static void iterate_4_step_mx_negative(float mn) {

    for (float mx = min; mx < negative_limit; mx /= factor_step) {
        iterate_5_step_arg(mn, mx);
    }
}
static void iterate_4_step_mx_zero(float mn) {

    iterate_5_step_arg(mn, 0.0f);
}
static void iterate_4_step_mx_positive(float mn) {

    for (float mx = positive_minimum; mx < positive_limit; mx *= factor_step) {
        iterate_5_step_arg(mn, mx);
    }
}
static void iterate_3_step_mn() {

    // Negative minimum
    for (float mn = negative_minimum; mn < negative_limit; mn /= factor_step) {

        iterate_4_step_mx_negative(mn);
        iterate_4_step_mx_zero(mn);
        iterate_4_step_mx_positive(mn);
    }

    // Zero minimum
    { float mn = 0.0f;

        iterate_4_step_mx_zero(mn);
        iterate_4_step_mx_positive(mn);
    }

    // Positive minimum
    for (float mn = positive_minimum; mn < positive_limit; mn *= factor_step) {

        iterate_4_step_mx_positive(mn);
    }
}

static void iterate_4_random_mx_negative(float mn) {

    for (float mx = min; mx < negative_limit; mx /= factor_step) {

        scilP_create_pattern_double(&dims, data, "random", mn, mx, 0.0f, 0.0f);
    }
}
static void iterate_4_random_mx_zero(float mn) {

    scilP_create_pattern_double(&dims, data, "random", mn, 0.0f, 0.0f, 0.0f);
}
static void iterate_4_random_mx_positive(float mn) {

    for (float mx = positive_minimum; mx < positive_limit; mx *= factor_step) {

        scilP_create_pattern_double(&dims, data, "random", mn, mx, 0.0f, 0.0f);
    }
}
static void iterate_3_random_mn() {

    // Negative minimum
    for (float mn = negative_minimum; mn < negative_limit; mn /= factor_step) {

        iterate_4_random_mx_negative(mn);
        iterate_4_random_mx_zero(mn);
        iterate_4_random_mx_positive(mn);
    }

    // Zero minimum
    { float mn = 0.0f;

        iterate_4_random_mx_zero(mn);
        iterate_4_random_mx_positive(mn);
    }

    // Positive minimum
    for (float mn = positive_minimum; mn < positive_limit; mn *= factor_step) {

        iterate_4_random_mx_positive(mn);
    }
}

static void iterate_3_constant_mn(){

    // Negative constant
    for (float mn = negative_minimum; mn < negative_limit; mn /= factor_step) {
        scilP_create_pattern_double(&dims, data, "constant", mn, 0.0f, 0.0f, 0.0f);
    }

    // Zero constant
    scilP_create_pattern_double(&dims, data, "constant", 0.0f, 0.0f, 0.0f, 0.0f);

    // Positive constant
    for (float mn = positive_minimum; mn < positive_limit; mn *= factor_step) {
        scilP_create_pattern_double(&dims, data, "constant", mn, 0.0f, 0.0f, 0.0f);
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
*/

static void iterate_4_algos(){

    uint8_t count = scil_compressors_available();
    for (uint8_t i = 0; i < count; ++i) {

        scilPr_initialize_user_hints(&hints);
        snprintf(hints.force_compression_methods, 8, "%d", i);
    }
}

// Random params
static void iterate_3_random_poly4(){

    float seed = (float)rand();
    float points = 1.0f + (float)(rand() % 63);

    scilP_create_pattern_double(&dims, data, "poly4", 0.0f, 1.0f, seed, points); // min and max don't matter


}
static void iterate_3_random_sin(){

    int iexp;
    do{
        iexp = rand() % 13 - 6;
    }while(iexp == 0);
    float height = powf(-10.0f + 20.0f * (float)rand()/RAND_MAX, (float)iexp);

    float octaves = 1.0f + (float)(rand() % 10);

    scilP_create_pattern_double(&dims, data, "sin", 0.0f, 1.0f, height, octaves); // min and max don't matter
}
static void iterate_3_random_steps(){

    int iexp;
    do{
        iexp = rand() % 13 - 6;
    }while(iexp == 0);
    float mx_base = 1.0f + 9.0f * (float)rand()/RAND_MAX;
    float mn_base = -mx_base;
    float offset = powf(-10.0f + 20.0f * (float)rand()/RAND_MAX, (float)iexp + rand() % 4 - 1);

    float mn = powf(mn_base, (float)iexp) + offset;
    float mx = powf(mx_base, (float)iexp) + offset;

    float arg = 2.0f + 98.0f * (float)rand()/RAND_MAX;

    scilP_create_pattern_double(&dims, data, "steps", mn, mx, arg, 0.0f);
}
static void iterate_3_random_random(){

    int iexp;
    do{
        iexp = rand() % 13 - 6;
    }while(iexp == 0);
    float mx_base = 1.0f + 9.0f * (float)rand()/RAND_MAX;
    float mn_base = -mx_base;
    float offset = powf(-10.0f + 20.0f * (float)rand()/RAND_MAX, (float)iexp + rand() % 4 - 1);

    float mn = powf(mn_base, (float)iexp) + offset;
    float mx = powf(mx_base, (float)iexp) + offset;

    scilP_create_pattern_double(&dims, data, "random", mn, mx, 0.0f, 0.0f);
}
static void iterate_3_random_constant(){

    float base = 1.0f + 9.0f * (float)rand()/RAND_MAX;

    int iexp;
    do{
        iexp = rand() % 13 - 6;
    }while(iexp == 0);

    float mn = powf(base, (float)iexp);

    scilP_create_pattern_double(&dims, data, "constant", mn, 0.0f, 0.0f, 0.0f);
}

static void iterate_2_random_patterns(size_t count){

    for (size_t i = 0; i < count; i++) {

        iterate_3_random_constant();
        iterate_3_random_random();
        iterate_3_random_steps();
        iterate_3_random_sin();
        iterate_3_random_poly4();
    }
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

        iterate_2_random_patterns(1);

        //free(dims);
        free(data);
    }
}

static void iterate_0_datasize(){

    const size_t min_count = 10;
    const size_t max_count = 100000001;

    for (size_t size = min_count; size < max_count; size *= 10) {
        iterate_1_dimensions(size);
    }
}

int main(void){

    benchmark_data(data, &dims);

    return 0;
}
