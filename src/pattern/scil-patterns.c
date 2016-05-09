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

#include <string.h>
#include <stdarg.h>

#include <scil-patterns.h>
#include <scil-pattern-internal.h>

#include <scil-internal.h>

#include <basic-patterns.h>
#include <simplex-noise.h>

static scil_pattern * patterns[] ={
  & scil_pattern_constant,
  & scil_pattern_rnd,
  & scil_pattern_steps,
  & scil_pattern_simplex_noise,
  NULL
};

int scilP_available_patterns_count(){
  static int count = -1;
  if(count == -1){
    for(count = 0; ; count++){
      if(patterns[count] == NULL){
        break;
      }
    }
  }
  return count;
}

char * scilP_available_patterns_name(int i){
  if (i >= 0 && i < scilP_available_patterns_count()){
    return patterns[i]->name;
  }
  return NULL;
}

int scilP_patterns_by_name(char * name){
  for(int i=0; i < scilP_available_patterns_count(); i++){
    if(strcmp(name, patterns[i]->name) == 0){
      return i;
    }
  }

  return -1;
}


int scilP_create_pattern_double(scil_dims * dims, double * buf, char * name, float mn, float mx, float arg){
  if (name == NULL){
    return SCIL_EINVAL;
  }
  int num = scilP_patterns_by_name(name);
  if (num == -1){
    return SCIL_EINVAL;
  }
  return patterns[num]->create(dims, buf, mn, mx, arg);
}

int scilP_create_pattern_float (scil_dims * dims, float * buffer, char * name,  float mn, float mx, float arg){
  size_t size = scil_get_data_size(SCIL_TYPE_DOUBLE, dims);
  double * buf = (double*) malloc(size);
  int ret = scilP_create_pattern_double(dims, buf, name, mn, mx, arg);
  if (ret != SCIL_NO_ERR){
    return ret;
  }
  for(size_t i=0; i < size; i++){
    buffer[i] = (float) buf[i];
  }
  free(buf);
  return SCIL_NO_ERR;
}

typedef struct{
  char * pattern;
  char * name;
  float mn;
  float mx;
  float arg;
  scilP_mutator * mutators; // apply those patterns after the creation
  int mutator_count;
} library_pattern;

static library_pattern * library = NULL;
static int library_size = 0;
static int library_capacity = 100;

static void library_add(char * pattern, char * name, float mn, float mx, float arg, int mutator_count, ...){
  assert(library_size < library_capacity);
  scilP_mutator * m = NULL;
  if (mutator_count > 0){
    va_list vl;
    va_start(vl,mutator_count);
    m = (scilP_mutator*) malloc(sizeof(void *) * mutator_count);
    for(int i=0; i < mutator_count; i++){
      m[i] = va_arg(vl, scilP_mutator);
    }
    va_end(vl);
  }
  library_pattern p = {pattern, name, mn, mx, arg, m, mutator_count};
  library[library_size] = p;

  library_size++;
}

static void create_library_patterns_if_needed(){
  static int initialized = 0;
  if (initialized){
    return;
  }
  initialized = 1;
  library = malloc(sizeof(library_pattern) * library_capacity);

  library_add("random", "randomIpolRep1-100", 1, 100, -1, 2, scilP_interpolator, scilP_repeater);

  library_add("constant", "constant0", 0, -1, -1, 0);
  library_add("constant", "constant35", 35.3335353, -1, -1, 0);

  library_add("random", "random0-1", 0, 1, -1, 0);
  library_add("random", "random1-100", 1, 100, -1, 0);
  library_add("random", "random-1-+1", -1, 1, -1, 0);

  library_add("steps", "steps2", 0, 1, 2, 0);
  library_add("steps", "steps100", 1, 100, 100, 0);

  library_add("simplexNoise", "simplex", -1, 1, 4711, 0);
}

int scilP_library_size(){
  create_library_patterns_if_needed();
  return library_size;
}

char * scilP_library_pattern_name(int p){
  create_library_patterns_if_needed();
  assert( p <= library_size && p >= 0);

  return library[p].name;
}

int scilP_library_create_pattern_double(int p, scil_dims * dims, double * buffer){
  create_library_patterns_if_needed();
  assert( p <= library_size && p >= 0);
  library_pattern * l = & library[p];
  int ret;
  ret = scilP_create_pattern_double(dims, buffer, l->pattern, l->mn, l->mx, l->arg);
  if (ret != SCIL_NO_ERR){
    return ret;
  }
  for(int i=0; i < l->mutator_count; i++){
    l->mutators[i](dims, buffer);
  }

  return ret;
}

int scilP_library_create_pattern_float (int p, scil_dims * dims, float * buffer){
  create_library_patterns_if_needed();
  assert( p <= library_size && p >= 0);
  size_t size = scil_get_data_size(SCIL_TYPE_DOUBLE, dims);
  double * buf = (double*) malloc(size);
  int ret = scilP_library_create_pattern_double(p, dims, buf);
  if (ret != SCIL_NO_ERR){
    return ret;
  }
  for(size_t i=0; i < size; i++){
    buffer[i] = (float) buf[i];
  }
  free(buf);
  return SCIL_NO_ERR;
}
