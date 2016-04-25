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

#include <scil-patterns.h>
#include <scil-pattern-internal.h>

#include <scil-internal.h>

#include <basic-patterns.h>

static scil_pattern * patterns[] ={
  & scil_pattern_constant,
  & scil_pattern_rnd,
  & scil_pattern_steps,
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
  size_t size = scilI_get_data_size(SCIL_TYPE_DOUBLE, dims);
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
