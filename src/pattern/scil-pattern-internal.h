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

#ifndef SCIL_PATTERN_INTERNAL_H
#define SCIL_PATTERN_INTERNAL_H

#include <scil-dims.h>

typedef struct scil_pattern {
    int (*create)(double* buffer, const scil_dims_t* dims, double mn, double mx, double arg, double arg2, int random_seed);
  char * name;
} scil_pattern_t;

typedef void (*scilP_mutator)(double* buffer, const scil_dims_t* dims, double arg);

void scilP_change_data_scale(double* buffer, const scil_dims_t* dims, double mn, double mx);

#endif // SCIL_PATTERN_INTERNAL_H
