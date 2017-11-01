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
#include <string.h>
#include <stdio.h>

#include <scil-hardware-limits.h>
#include <scil-error.h>

static float hardware_limits[HARDWARE_MAX];
static const char* hardware_names[] = {
  "storage",
  "network",
  NULL
};

void scilU_initialize_hardware_limits(){
  memset(hardware_limits, 0, sizeof(float)* HARDWARE_MAX);
}

int scilU_add_hardware_limit(const char* name, const char* str){
  char** pos = (char**) hardware_names;
  int i = 0;
  while(*pos != NULL){
    if(strcmp(*pos, name) == 0){
      // found it
      hardware_limits[i] = (float) atof(str);
      return SCIL_NO_ERR;
    }
    i++;
    pos++;
  }
  return SCIL_EINVAL;
}
