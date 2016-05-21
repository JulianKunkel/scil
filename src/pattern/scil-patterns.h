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

#ifndef SCIL_PATTERN_LIB_H
#define SCIL_PATTERN_LIB_H

#include <scil.h>

int scilP_available_patterns_count();
char * scilP_available_patterns_name(int p);

/*
 * Create the pattern selected by num with the arguments
 */
int scilP_create_pattern_double(scil_dims * dims, double * buffer, char * name, float mn, float mx, float arg, float arg2);
int scilP_create_pattern_float (scil_dims * dims, float * buffer, char * name, float mn, float mx, float arg, float arg2);

/*
 * The pattern library contains a list of useful patterns.
 */
int scilP_library_size();
char * scilP_library_pattern_name(int pattern);

int scilP_library_create_pattern_double(int pattern, scil_dims * dims, double * buffer);
int scilP_library_create_pattern_float (int pattern, scil_dims * dims, float * buffer);

#endif
