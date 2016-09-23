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
    int (*create)(double* const buffer, const scil_dims* const dims, float mn, float mx, float arg, float arg2);
  char * name;
} scil_pattern_t;

typedef void (*scilPa_mutator)(double* const buffer, const scil_dims* const dims, float arg);

void scilPI_change_scale(double* const buffer, const scil_dims* const dims, float mn, float mx);

#endif // SCIL_PATTERN_INTERNAL_H
