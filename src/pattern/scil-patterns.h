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

#ifndef SCIL_PATTERN_H
#define SCIL_PATTERN_H

#include <scil-dims.h>

int   scilPa_get_available_patterns_count();
char* scilPa_get_pattern_name(int index);
int   scilPa_get_pattern_index(const char* name);

/*
 * Create the pattern selected by num with the arguments
 */
int scilPa_create_pattern_double(double* buffer,
                                 const scil_dims* dims,
                                 const char* name,
                                 float mn,
                                 float mx,
                                 float arg,
                                 float arg2);

int scilPa_create_pattern_float (float*  const buffer,
                                 const scil_dims* dims,
                                 const char* name,
                                 float mn,
                                 float mx,
                                 float arg,
                                 float arg2);

/*
 * The pattern library contains a list of useful patterns.
 */
int scilPa_get_pattern_library_size();
char* scilPa_get_library_pattern_name(int pattern);

int scilPa_create_library_pattern_double(double* buffer, const scil_dims* dims, int pattern);
int scilPa_create_library_pattern_float (float*  const buffer, const scil_dims* dims, int pattern);

#endif // SCIL_PATTERN_H
