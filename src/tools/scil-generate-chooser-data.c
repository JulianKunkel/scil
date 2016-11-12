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

#include <scil-compressors.h>
#include <scil-algo-chooser.h>
#include <scil-internal.h>
#include <scil-patterns.h>
#include <scil-util.h>

#define allocate(type, name, count) type* name = (type*)malloc(count * sizeof(type))

static enum metrics {
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

static int get_data_characteristics(const double* data, size_t count,
                                    double* minimum   , double* maximum,
                                    double* mean      , double* median,
                                    double* std_deviation){

    *minimum       = get_data_minimum(data, count);
    *maximum       = get_data_maximum(data, count);
    *mean          = get_data_mean(data, count);
    *median        = get_data_median(data, count);
    *std_deviation = get_data_std_deviation(data, count, *mean);

    return 0;
}

static int is_str_in_array(const char** strarr, int count){
    for (size_t i = 0; i < count; i++) {
        if (strcmp(strarr[i], ))
            continue;
        return 1;
    }
    return 0;
}

int main(int argc, char** argv){

    char** metrics = &argv[1];



    return 0;
}
