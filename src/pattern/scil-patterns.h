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
#include <scil-datatypes.h>

void scilP_set_random_seed(int val);

int   scilP_get_available_patterns_count();
char* scilP_get_pattern_name(int index);
int   scilP_get_pattern_index(const char* name);

/*
 * Create the pattern selected by num with the arguments
 */
int scilP_create_pattern_double(double * buffer,
                                 const scil_dims_t* dims,
                                 const char* name,
                                 double mn,
                                 double mx,
                                 double arg,
                                 double arg2);

int scilP_create_pattern(void * buffer, SCIL_Datatype_t datatype,
                                const scil_dims_t* dims,
                                const char* name,
                                double mn,
                                double mx,
                                double arg,
                                double arg2);

void scilP_convert_data_from_double(void * out, SCIL_Datatype_t datatype,  double * in, const scil_dims_t* dims);

/*
 * The pattern library contains a list of useful patterns.
 */
int scilP_get_pattern_library_size();
char* scilP_get_library_pattern_name(int pattern);

int scilP_create_library_pattern(void * buffer, SCIL_Datatype_t datatype, const scil_dims_t* dims, int pattern);

#endif // SCIL_PATTERN_H
