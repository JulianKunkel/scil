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

// #############################################################################
// # Metrics Default Values
// #############################################################################

#define DEFAULT_ABSOLUTE_ERROR_TOLERANCE  0.005

#define DEFAULT_RELATIVE_ERROR_PERCENTAGE 1.0
#define DEFAULT_SIGNIFICANT_DIGITS        2
#define DEFAULT_SIGNIFICANT_BITS          7

// #############################################################################
// # Data Characteristics Aquistion
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

    *maximum = max;
}

static double get_data_mean(const double* const data, size_t count){

      double mn = 0.0;

      for (size_t i = 0; i < count; i++) {
          mn += data[i];
      }

      return mn / count;
}

static int compdblp(const void* const a, const void* const b){
    if (*(double*)a > *(double*)b){ return 1; }
    if (*(double*)a < *(double*)b){ return -1; }
    return 0;
}
static double get_data_median(const double* const data, size_t count){

    allocate(double, tmp_buf, count);
    memcpy(tmp_buf, data, count);

    qsort(tmp_buf, count, sizeof(double) * count, compdblp);

    double median;

    if (count % 2 == 0) { median = (tmp_buff[count/2] + tmp_buf[count/2 + 1]) / 2; }
    else                { median = tmp_buf[count/2]; }

    free(tmp_buf);

    return median;
}

static double get_standard_deviation(const double* const data, size_t count, double mean){

    double variance = 0.0;

    for (size_t i = 0; i < count; i++) {
        double dif = data[i] - mean;
        variance += dif * dif;
    }

    return sqrt(variance);
}

static int get_data_characteristics(const double* const data, size_t count,
                                    double* const minimum   , double* const maximum,
                                    double* const mean      , double* const median,
                                    double* const standard_deviation){

    *minimum            = get_data_minimum(data, count);
    *maximum            = get_data_maximum(data, count);
    *mean               = get_data_mean(data, count);
    *median             = get_data_median(data, count);
    *standard_deviation = get_data_standard_deviation(data, count, *mean);

    return 1;
}

// #############################################################################
// # Data Generation
// #############################################################################

static int generate_single_metric_data(const char[] metric, double start, double step, double end){



    return 1;
}

int main(int argc, char** argv){



    return 0;
}
