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
#include <math.h>

#include <scil-patterns.h>
#include <scil-pattern-internal.h>

#include <scil-error.h>
#include <scil-debug.h>
#include <scil-util.h>

#include <basic-patterns.h>
#include <basic-mutators.h>
#include <simplex-noise.h>

static scil_pattern_t * patterns[] ={
  & scil_pattern_constant,
  & scil_pattern_rnd,
  & scil_pattern_steps,
  & scil_pattern_sin,
  & scil_pattern_simplex_noise,
  & scil_pattern_poly4,
  NULL
};

typedef struct{
  scilP_mutator call;
  float arg;
} mutator_config;

typedef struct{
  char * pattern;
  char * name;
  float mn;
  float mx;
  float arg;
  float arg2;
  mutator_config * mutators; // apply those patterns after the creation

  int mutator_count;
} library_pattern;

static library_pattern * library = NULL;
static int library_size = 0;
static int library_capacity = 100;
static int library_seed = 0;

void scilP_set_random_seed(int val){
  library_seed  = val;
}

int scilP_get_available_patterns_count()
{
    static int count = -1;

    if (count != -1)
        return count;

    for(count = 0; ; count++){
        if(patterns[count] == NULL){
            break;
        }
    }
    return count;
}

char* scilP_get_pattern_name(int index)
{
    if (index < 0 || index >= scilP_get_available_patterns_count())
        return NULL;

    return patterns[index]->name;
}

int scilP_get_pattern_index(const char* name)
{
    for(int i=0; i < scilP_get_available_patterns_count(); i++){
        if(strcmp(name, patterns[i]->name) == 0){
            return i;
        }
    }
    return -1;
}

int scilP_create_pattern_double(double * buffer, const scil_dims_t* dims, const char* name, double mn, double mx, double arg, double arg2){
  if (name == NULL){
    return SCIL_EINVAL;
  }
  int num = scilP_get_pattern_index(name);
  if (num == -1){
    return SCIL_EINVAL;
  }
  return patterns[num]->create(buffer, dims, mn, mx, arg, arg2, library_seed);
}

static void library_add(char * pattern, char * name, float mn, float mx, float arg, float arg2, int mutator_count, ...){
  assert(library_size < library_capacity);
  mutator_config * m = NULL;

  assert(! scilU_float_equal(mn, mx));

  if (mutator_count > 0){
    va_list vl;
    va_start(vl,mutator_count);
    m = (mutator_config*) malloc(sizeof(mutator_config) * mutator_count);
    for(int i=0; i < mutator_count; i++){
      m[i].call = va_arg(vl, scilP_mutator);
      m[i].arg = (float) va_arg(vl, double);
    }
    va_end(vl);
  }

  library_pattern p = {pattern, name, mn, mx, arg, arg2, m, mutator_count};
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

  library_add("random", "randomRep10-100", 1, 100,  0, -1,     1, scilP_repeater, 10.0);
  library_add("random", "randomRep2-100", 1, 100,  0, -1,      1, scilP_repeater, 2.0);

  library_add("random", "randomRep10-1-+1", -1, 1,  0,  -1,    1, scilP_repeater, 10.0);
  library_add("random", "randomRep2-1-+1", -1, 1,   0, -1,     1, scilP_repeater, 2.0);

  library_add("random", "randomIpol10-100", 1, 100, 0,  -1,   1, scilP_interpolator, 10.0);
  library_add("random", "randomIpol2-100", 1, 100,  0, -1,    1, scilP_interpolator, 2.0);

  library_add("random", "randomIpol10-1-+1", -1, 1, 0,  -1,   1, scilP_interpolator, 10.0);
  library_add("random", "randomIpol2-1-+1", -1, 1,  0, -1,    1, scilP_interpolator, 2.0);

  library_add("constant", "constant0", 0, -1, -1, 0, 0);
  library_add("constant", "constant35", 35.3335353, -1, -1, 0, 0);

  library_add("random", "random0-1", 0, 1, 0,  -1, 0);
  library_add("random", "random1-100", 1, 100, 0,  -1, 0);
  library_add("random", "random-1-+1", -1, 1, 0,  -1, 0);

  library_add("steps", "steps2", 0, 100, 2, 0, 0);
  library_add("steps", "steps16", 0, 100, 16, 0, 0);
  library_add("steps", "steps100", 1, 100, 100, 0, 0);

  library_add("sin", "sin11", 1, 100, 1.0, 1, 0); // 1 pass
  library_add("sin", "sin16", 0, 100, 1.0, 6, 0); // 6 passes
  library_add("sin", "sin31", 1, 100, 3.0, 1, 0); // 3 sines
  library_add("sin", "sin35", 0, 100, 3.0, 5, 0); // 3 sines, 5 passes
  library_add("sin", "sin23", 1, 100, 2.0, 3, 0); // 3 passes, 2 sines

  library_add("poly4", "poly4--1-5", 1, 100, -1, 5, 0);
  library_add("poly4", "poly4-234-3", 0, 100, 234, 3, 0);
  library_add("poly4", "poly4-65432-14", 0, 100, 65432, 14, 0);

  library_add("simplexNoise", "simplex102", 0, 100, 1.0, 2, 0); // 2 passes
  library_add("simplexNoise", "simplex106", 0, 100, 1.0, 6, 0); // 6 passes
  library_add("simplexNoise", "simplex206", 0, 100, 3.0, 6, 0); // 6 passes, 3 hills
}

int scilP_get_pattern_library_size(){
  create_library_patterns_if_needed();
  return library_size;
}

char * scilP_get_library_pattern_name(int p){
  create_library_patterns_if_needed();
  assert( p <= library_size && p >= 0);

  return library[p].name;
}

void scilP_convert_data_from_double(void * buffer, SCIL_Datatype_t datatype,  double * data, const scil_dims_t* dims){
  size_t elemCount = scil_dims_get_count(dims);

	switch(datatype){
		case(SCIL_TYPE_FLOAT):{
      float * buffer_real = (float*)buffer;
			for(unsigned x = 0; x < elemCount; x++){
				buffer_real[x] = (float) data[x];
			}
			break;
		}
	  case(SCIL_TYPE_INT8):{
			int8_t * buffer_real = (int8_t*) buffer;
			for(unsigned x = 0; x < elemCount; x++){
				buffer_real[x] = (int8_t) round(data[x]);
			}
			break;
		}
	  case(SCIL_TYPE_INT16):{
			int16_t * buffer_real = (int16_t*)  buffer;
			for(unsigned x = 0; x < elemCount; x++){
				buffer_real[x] = (int16_t) round(data[x]);
			}
			break;
		}
	  case(SCIL_TYPE_INT32):{
			int32_t * buffer_real = (int32_t*)  buffer;
			for(unsigned x = 0; x < elemCount; x++){
				buffer_real[x] = (int32_t) round(data[x]);
			}
			break;
		}
	  case(SCIL_TYPE_INT64):{
			int64_t * buffer_real = (int64_t*)  buffer;
			for(unsigned x = 0; x < elemCount; x++){
				buffer_real[x] = (int64_t) round(data[x]);
			}
			break;
		}
		default:
			assert(0 && "Should never be here");
	}
}


int scilP_create_pattern(void * buffer, SCIL_Datatype_t datatype, const scil_dims_t* dims, const char* name, double mn, double mx, double arg, double arg2){
  double * data;
  if (datatype != SCIL_TYPE_DOUBLE){
    int doubleSize = scil_get_compressed_data_size_limit(dims, datatype);
    data = malloc(doubleSize * sizeof(double));
  }else{
    data = (double*) buffer;
  }

  int ret;
  ret = scilP_create_pattern_double(data, dims, name, mn, mx, arg, arg2);
  if (ret != SCIL_NO_ERR){
    if (datatype != SCIL_TYPE_DOUBLE){
      free(data);
    }
    return ret;
  }

  if (datatype != SCIL_TYPE_DOUBLE){
    scilP_convert_data_from_double(buffer, datatype,  data, dims);

    free(data);
  }
  return SCIL_NO_ERR;
}

int scilP_create_library_pattern(void * buffer, SCIL_Datatype_t datatype, const scil_dims_t* dims, int pattern_index)
{
  double * data;
  if (datatype != SCIL_TYPE_DOUBLE){
    int doubleSize = scil_get_compressed_data_size_limit( dims, SCIL_TYPE_DOUBLE);
    data = malloc(doubleSize);
  }else{
    data = (double*) buffer;
  }

  create_library_patterns_if_needed();
  assert(pattern_index <= library_size && pattern_index >= 0);
  library_pattern* l = &library[pattern_index];
  int ret;
  ret = scilP_create_pattern_double(data, dims, l->pattern, l->mn, l->mx, l->arg, l->arg2);
  if (ret != SCIL_NO_ERR){
    if (datatype != SCIL_TYPE_DOUBLE){
      free(data);
    }
    return ret;
  }
  for(int i=0; i < l->mutator_count; i++){
    mutator_config* m = &l->mutators[i];
    m->call(data, dims, m->arg);
  }

  if (datatype != SCIL_TYPE_DOUBLE){
    scilP_convert_data_from_double(buffer, datatype,  data, dims);

    free(data);
  }

  return ret;
}


void scilP_change_data_scale(double* buffer, const scil_dims_t* dims, double mn, double mx){
  // fix min + max, first identify min/max
  size_t count = scil_dims_get_count(dims);
  double mn_o = 1e308, mx_o=-1e308;
  for (size_t i=0; i < count; i++){
    mn_o = min(mn_o, buffer[i]);
    mx_o = max(mx_o, buffer[i]);
  }

  double scaling = (double)(mx - mn) / (mx_o - mn_o); // intended min/max
  // rescale
  for (size_t i=0; i < count; i++){
    buffer[i] = (double) mn + (buffer[i]-mn_o) *scaling;
  }
}
