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
#include <pthread.h>

#include <scil.h>
#include <scil-algo-chooser.h>
#include <scil-debug.h>
#include <scil-patterns.h>
#include <scil-util.h>

#define allocate(type, name, count) type* name = (type*)malloc(count * sizeof(type))

enum metrics {
    DATA_SIZE,
    ELEMENT_COUNT,
    DIMENSIONALITY,
    MINIMUM,
    MAXIMUM,
    MEAN,
    MEDIAN,
    STANDARD_DEVIATION,
    MAX_STEP_SIZE,
    ABS_ERR_TOL,
    REL_ERR_TOL
};

static const char *available_metrics[] = {
    "dtype",
    "ecount",
    "dsize",
    "dim",
    "min",
    "max",
    "mean",
    "stddev",
    "maxstp",
    "abserr",
    "relerr",
    NULL
};

#define DEFAULT_DTYPE   SCIL_TYPE_DOUBLE
#define DEFAULT_ECOUNT  1024
#define DEFAULT_DIM     1
#define DEFAULT_MIN     -1024
#define DEFAULT_MAX     1024
#define DEFAULT_ABS_ERR 0.005
#define DEFAULT_REL_ERR 1

#define SAMPLE_SIZE 10000

#define FILE_NAME "machine_learning_data8.csv"
static FILE *file = NULL;

typedef struct line_data {
    size_t line;
    char algo[16];
    char pattern[16];
    float pat_param_min;
    float pat_param_max;
    float pat_param_arg1;
    float pat_param_arg2;
    size_t size;
    size_t count;
    uint8_t dims;
    size_t dim1;
    size_t dim2;
    size_t dim3;
    size_t dim4;
    double min;
    double max;
    double mean;
    double median;
    double stddev;
    double maxstep;
    double abs_tol;
    double rel_tol;
    double compthru;
    double decompthru;
    double compratio;
} line_data_t;

static line_data_t current_data = { 0, "", "", 0.0, 0.0, 0.0, 0.0, 0, 0, 0, 0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };

static void write_line(){
    printf("%lu,%s,%s,%f,%f,%f,%f,%lu,%lu,%u,%lu,%lu,%lu,%lu,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f\n",
        current_data.line,
        current_data.algo,
        current_data.pattern,
        (double)current_data.pat_param_min,
        (double)current_data.pat_param_max,
        (double)current_data.pat_param_arg1,
        (double)current_data.pat_param_arg2,
        current_data.size,
        current_data.count,
        current_data.dims,
        current_data.dim1,
        current_data.dim2,
        current_data.dim3,
        current_data.dim4,
        current_data.min,
        current_data.max,
        current_data.mean,
        current_data.median,
        current_data.stddev,
        current_data.maxstep,
        current_data.abs_tol,
        current_data.rel_tol,
        current_data.compthru,
        current_data.decompthru,
        current_data.compratio);

    fprintf(file, "%lu,%s,%s,%f,%f,%f,%f,%lu,%lu,%u,%lu,%lu,%lu,%lu,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f\n",
        current_data.line,
        current_data.algo,
        current_data.pattern,
        (double)current_data.pat_param_min,
        (double)current_data.pat_param_max,
        (double)current_data.pat_param_arg1,
        (double)current_data.pat_param_arg2,
        current_data.size,
        current_data.count,
        current_data.dims,
        current_data.dim1,
        current_data.dim2,
        current_data.dim3,
        current_data.dim4,
        current_data.min,
        current_data.max,
        current_data.mean,
        current_data.median,
        current_data.stddev,
        current_data.maxstep,
        current_data.abs_tol,
        current_data.rel_tol,
        current_data.compthru,
        current_data.decompthru,
        current_data.compratio);

    current_data.line++;
}

//#define AVAILABLE_COMPRESSION_CHAINS_COUNT 15
//static const *available_compression_chains[AVAILABLE_COMPRESSION_CHAINS_COUNT] = {
//
//}

// #############################################################################
// # Utility Functions
// #############################################################################

static size_t get_index_2d(size_t x, size_t y, const scil_dims_t *dims){
    return y * dims->length[0] + x;
}
static size_t get_index_3d(size_t x, size_t y, size_t z, const scil_dims_t *dims){
    return (z * dims->length[1] + y) * dims->length[0] + x;
}
static size_t get_index_4d(size_t x, size_t y, size_t z, size_t w, const scil_dims_t *dims){
    return ((w * dims->length[2] + z) * dims->length[1] + y) * dims->length[0] + x;
}

static int in_strarr(const char* string, const char* const* strarr, size_t count){

    for (size_t i = 0; i < count; i++) {
        if (strcmp(strarr[i], string))
            continue;
        return 1;
    }
    return 0;
}

static int get_metric_bit_mask(const char *const *metric_args, size_t count){
    int result = 0;

    const char *const *current_metric = available_metrics;
    size_t i = 0;
    while(*current_metric != NULL){
        //printf("%s\n", *current_metric);
        result |= in_strarr(*current_metric, metric_args, count) << i;
        i++;
        current_metric++;
    }

    return result;
}

static double get_random_double_in_range(double minimum, double maximum){

    return minimum + (maximum - minimum) * (double)rand()/RAND_MAX;
}

static int get_random_integer_in_range(int minimum, int maximum){

    assert(maximum >= minimum);

    if (minimum == maximum) return minimum;

    return minimum + rand()%(maximum - minimum + 1);
}

// #############################################################################
// # Data Characteristics Aquisition
// #############################################################################

static double get_data_minimum(const double* data, size_t count){

    double min = INFINITY;

    for (size_t i = 0; i < count; i++) {
        if (data[i] < min) { min = data[i]; }
    }

    return min;
}

static double get_data_maximum(const double* data, size_t count){

    double max = -INFINITY;

    for (size_t i = 0; i < count; i++) {
        if (data[i] > max) { max = data[i]; }
    }

    return max;
}

static double get_data_mean(const double* data, size_t count){

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
static double get_data_median(const double* data, size_t count){

    allocate(double, tmp_buf, count);
    memcpy(tmp_buf, data, count * sizeof(double));

    qsort(tmp_buf, count, sizeof(double), compdblp);

    double median = 0.0;

    if (count % 2 == 0) { median = 0.5 * (tmp_buf[count/2] + tmp_buf[count/2 + 1]); }
    else                { median = tmp_buf[count/2]; }

    free(tmp_buf);

    return median;
}

static double get_data_std_deviation(const double* data, size_t count, double mean){

    double variance_sum = 0.0;

    for (size_t i = 0; i < count; i++) {
        double dif = data[i] - mean;
        variance_sum += dif * dif;
    }

    return sqrt(variance_sum / count);
}

static double get_data_std_deviation_alt(const double* data, size_t count){

    double sum = 0.0, squared_sum = 0.0;

    for (size_t i = 0; i < count; i++) {
        sum         += data[i];
        squared_sum += data[i] * data[i];
    }

    return sqrt((squared_sum - (sum * sum) / count) / count);
}

static double get_data_max_step_1d(const double *data, const scil_dims_t *dims){

    double max_step = 0.0;

    size_t xsize = dims->length[0];

    for (size_t x = 1; x < xsize; x++) {
        double step = fabs(data[x] - data[x-1]);
        if (max_step < step) { max_step = step; }
    }

    return max_step;
}
static double get_data_max_step_2d(const double *data, const scil_dims_t *dims){

    double max_step = 0.0;

    size_t xsize = dims->length[0];
    size_t ysize = dims->length[1];

    // X-direction
    for (size_t y = 0; y < ysize; y++) {
        for (size_t x = 1; x < xsize; x++) {
            size_t i1 = get_index_2d(x, y, dims);
            size_t i2 = get_index_2d(x - 1, y, dims);
            double step = fabs(data[i1] - data[i2]);
            if (max_step < step) { max_step = step; }
        }
    }

    // Y-direction
    for (size_t y = 1; y < ysize; y++) {
        for (size_t x = 0; x < xsize; x++) {
            size_t i1 = get_index_2d(x, y, dims);
            size_t i2 = get_index_2d(x, y-1, dims);
            double step = fabs(data[i1] - data[i2]);
            if (max_step < step) { max_step = step; }
        }
    }

    return max_step;
}
static double get_data_max_step_3d(const double *data, const scil_dims_t *dims){

    double max_step = 0.0;

    size_t xsize = dims->length[0];
    size_t ysize = dims->length[1];
    size_t zsize = dims->length[2];

    // X-direction
    for (size_t z = 0; z < zsize; z++) {
        for (size_t y = 0; y < ysize; y++) {
            for (size_t x = 1; x < xsize; x++) {
                size_t i1 = get_index_3d(x, y, z, dims);
                size_t i2 = get_index_3d(x-1, y, z, dims);
                double step = fabs(data[i1] - data[i2]);
                if (max_step < step) { max_step = step; }
            }
        }
    }
    // Y-direction
    for (size_t z = 0; z < zsize; z++) {
        for (size_t y = 1; y < ysize; y++) {
            for (size_t x = 0; x < xsize; x++) {
                size_t i1 = get_index_3d(x, y, z, dims);
                size_t i2 = get_index_3d(x, y-1, z, dims);
                double step = fabs(data[i1] - data[i2]);
                if (max_step < step) { max_step = step; }
            }
        }
    }
    // Z-direction
    for (size_t z = 1; z < zsize; z++) {
        for (size_t y = 0; y < ysize; y++) {
            for (size_t x = 0; x < xsize; x++) {
                size_t i1 = get_index_3d(x, y, z, dims);
                size_t i2 = get_index_3d(x, y, z-1, dims);
                double step = fabs(data[i1] - data[i2]);
                if (max_step < step) { max_step = step; }
            }
        }
    }

    return max_step;
}
static double get_data_max_step_4d(const double *data, const scil_dims_t *dims){

    double max_step = 0.0;

    size_t xsize = dims->length[0];
    size_t ysize = dims->length[1];
    size_t zsize = dims->length[2];
    size_t wsize = dims->length[3];

    // X-direction
    for (size_t w = 0; w < wsize; w++) {
        for (size_t z = 0; z < zsize; z++) {
            for (size_t y = 0; y < ysize; y++) {
                for (size_t x = 1; x < xsize; x++) {
                    size_t i1 = get_index_4d(x, y, z, w, dims);
                    size_t i2 = get_index_4d(x-1, y, z, w, dims);
                    double step = fabs(data[i1] - data[i2]);
                    if (max_step < step) { max_step = step; }
                }
            }
        }
    }
    // Y-direction
    for (size_t w = 0; w < wsize; w++) {
        for (size_t z = 0; z < zsize; z++) {
            for (size_t y = 1; y < ysize; y++) {
                for (size_t x = 0; x < xsize; x++) {
                    size_t i1 = get_index_4d(x, y, z, w, dims);
                    size_t i2 = get_index_4d(x, y-1, z, w, dims);
                    double step = fabs(data[i1] - data[i2]);
                    if (max_step < step) { max_step = step; }
                }
            }
        }
    }
    // Z-direction
    for (size_t w = 0; w < wsize; w++) {
        for (size_t z = 1; z < zsize; z++) {
            for (size_t y = 0; y < ysize; y++) {
                for (size_t x = 0; x < xsize; x++) {
                    size_t i1 = get_index_4d(x, y, z, w, dims);
                    size_t i2 = get_index_4d(x, y, z-1, w, dims);
                    double step = fabs(data[i1] - data[i2]);
                    if (max_step < step) { max_step = step; }
                }
            }
        }
    }
    // W-direction
    for (size_t w = 1; w < wsize; w++) {
        for (size_t z = 0; z < zsize; z++) {
            for (size_t y = 0; y < ysize; y++) {
                for (size_t x = 0; x < xsize; x++) {
                    size_t i1 = get_index_4d(x, y, z, w, dims);
                    size_t i2 = get_index_4d(x, y, z, w-1, dims);
                    double step = fabs(data[i1] - data[i2]);
                    if (max_step < step) { max_step = step; }
                }
            }
        }
    }

    return max_step;
}
static double get_data_max_step(const double *data, const scil_dims_t *dims){

    switch (dims->dims){
        case 1: return get_data_max_step_1d(data, dims);
        case 2: return get_data_max_step_2d(data, dims);
        case 3: return get_data_max_step_3d(data, dims);
        case 4: return get_data_max_step_4d(data, dims);
    }
}

static int set_data_characteristics(const double *data, const scil_dims_t *dims){

    current_data.min    = get_data_minimum(data, current_data.count);
    current_data.max    = get_data_maximum(data, current_data.count);
    current_data.mean   = get_data_mean(data, current_data.count);
    current_data.median = get_data_median(data, current_data.count);
    current_data.stddev = get_data_std_deviation(data, current_data.count, current_data.mean);
    current_data.maxstep = get_data_max_step(data, dims);

    return 0;
}

// #############################################################################
// # Data Generation
// #############################################################################

static void evaluate_compression_algorithm(double *buffer, scil_dims_t *dims, char algo){

    scil_user_hints_t hints;
    scil_user_hints_initialize(&hints);

    hints.force_compression_methods  = strndup(&algo, 1);
    hints.absolute_tolerance         = current_data.abs_tol;
    hints.relative_tolerance_percent = current_data.rel_tol;

    switch (algo) {
        case '0': strncpy(current_data.algo, "memcpy"       , 16); break;
        case '1': strncpy(current_data.algo, "abstol"       , 16); break;
        case '2': strncpy(current_data.algo, "gzip"         , 16); break;
        case '3': strncpy(current_data.algo, "sigbits"      , 16); break;
        case '4': strncpy(current_data.algo, "fpzip"        , 16); break;
        case '5': strncpy(current_data.algo, "zfp_abstol"   , 16); break;
        case '6': strncpy(current_data.algo, "zfp_precision", 16); break;
        case '7': strncpy(current_data.algo, "lz4fast"      , 16); break;
    }

    scil_context_t* ctx;
    scil_context_create(&ctx, SCIL_TYPE_DOUBLE, 0, NULL, &hints);

    size_t source_size = scil_dims_get_size(dims, SCIL_TYPE_DOUBLE);
    size_t dest_size   = scil_get_compressed_data_size_limit(dims, SCIL_TYPE_DOUBLE);
    byte* dest         = (byte*)scilU_safe_malloc(dest_size);

    // Compression analysis
    clock_t start = clock();
    int ret = scil_compress(dest, dest_size, buffer, dims, &dest_size, ctx);
    clock_t end = clock();

    if (ret)
    {
      printf("Compression error\n");
      return;
    }

    current_data.compthru = 1e-6 * source_size * CLOCKS_PER_SEC / (end - start); // MB/s

    current_data.compratio = (double)source_size / dest_size;

    double *decompd = (double *)scilU_safe_malloc(source_size);
    byte *temp = (byte *)scilU_safe_malloc(dest_size);

    // Decompression analysis
    start = clock();
    ret = scil_decompress(SCIL_TYPE_DOUBLE, decompd, dims, dest, dest_size, temp);
    end = clock();

    if (ret)
    {
      printf("Decompression error\n");
      return;
    }

    current_data.decompthru = 1e-6 * source_size * CLOCKS_PER_SEC / (end - start); // MB/s

    write_line();

    scil_destroy_context(ctx);
    free(dest);
    free(decompd);
    free(temp);
}

static void evaluate_compression_algorithms(double *buffer, scil_dims_t *dims){

    for (uint8_t i = 0; i < 8; i++) {
        char c = '0' + (char)i;
        evaluate_compression_algorithm(buffer, dims, c);
    }
}

static void generate_data(){

    // Pattern name
    uint8_t pid = rand() % 5;
    switch(pid){
        case 0: strncpy(current_data.pattern, "constant"    , 16); break;
        case 1: strncpy(current_data.pattern, "random"      , 16); break;
        case 2: strncpy(current_data.pattern, "steps"       , 16); break;
        case 3: strncpy(current_data.pattern, "sin"         , 16); break;
        case 4: strncpy(current_data.pattern, "simplexNoise", 16); break;
    }

    // Dimensionality
    current_data.dims = (uint8_t)get_random_integer_in_range(1, 4);
    size_t e_count = (size_t)pow(2.0, get_random_double_in_range(8.0, 22.0)); // maximum of 32 MB for double values
    size_t side = (size_t)pow(e_count, 1.0/current_data.dims);

    current_data.dim1 = side;
    current_data.dim2 = current_data.dims > 1 ? side : 1;
    current_data.dim3 = current_data.dims > 2 ? side : 1;
    current_data.dim4 = current_data.dims > 3 ? side : 1;

    scil_dims_t dims;
    switch(current_data.dims){
        case 1: scil_dims_initialize_1d(&dims, side); break;
        case 2: scil_dims_initialize_2d(&dims, side, side); break;
        case 3: scil_dims_initialize_3d(&dims, side, side, side); break;
        case 4: scil_dims_initialize_4d(&dims, side, side, side, side); break;
    }

    current_data.size  = scil_dims_get_size(&dims, SCIL_TYPE_DOUBLE);
    current_data.count = scil_dims_get_count(&dims);

    allocate(double, data_buffer, current_data.count);

    // Minimum and maximum
    double point_a = pow(2.0, get_random_double_in_range(-14, 14));
    double point_b = pow(2.0, get_random_double_in_range(-14, 14));

    point_a = rand() % 2 == 1 ? -point_a : point_a;
    point_b = rand() % 2 == 1 ? -point_b : point_b;

    if (point_b > point_a) {
        current_data.pat_param_min = point_a;
        current_data.pat_param_max = point_b;
    }
    else {
        current_data.pat_param_max = point_b;
        current_data.pat_param_min = point_a;
    }

    // Other Arguments
    current_data.pat_param_arg1 = get_random_double_in_range(1, 16);
    current_data.pat_param_arg2 = get_random_double_in_range(1, 16);

    printf("Generating buffer of %lu values with the %s pattern... ", current_data.count, current_data.pattern);
    fflush(stdout);

    if (pid == 0) {
        current_data.pat_param_min = point_a;
        current_data.pat_param_max = -INFINITY;
    }

    scilP_create_pattern_double(data_buffer,
                                 &dims,
                                 current_data.pattern,
                                 current_data.pat_param_min,
                                 current_data.pat_param_max,
                                 current_data.pat_param_arg1,
                                 current_data.pat_param_arg2);

    printf("Done!\n");

    // Data characteristics
    set_data_characteristics(data_buffer, &dims);

    // User Params for compression
    current_data.abs_tol = pow(2.0, get_random_double_in_range(-13, 2));
    current_data.rel_tol = pow(2.0, get_random_double_in_range(-10, 4));

    evaluate_compression_algorithms(data_buffer, &dims);

    free(data_buffer);
}

// #############################################################################
// # Main Function
// #############################################################################

int main(int argc, char** argv){

    srand((unsigned)time(NULL));

    file = fopen(FILE_NAME, "w");
    if(file == NULL){
        fprintf(stderr, "%s\n", "Error, opening file.");
        return 1;
    }

    fprintf(file, "%s\n", "Index,Algorithm,Pattern Name,Pattern Param Minimum,Pattern Param Maximum,Pattern Param 1,Pattern Param 2,Size of buffer,Value count,Dimensionality,Count x-Dim,Count y-Dim,Count z-Dim,Count w-Dim,Minimum,Maximum,Average,Median,Standard deviation,Maximum step,Absolute error tolerance,Relative error tolerance,Compression throughput,Decompression throughput,Compression ratio");

    for (size_t i = 0; i < SAMPLE_SIZE; i++){
        generate_data();
        if (i % 12 == 0) { srand(time(NULL)); }
    }

    fclose(file);

    return 0;
}
