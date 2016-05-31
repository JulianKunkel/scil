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

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <scil-patterns.h>
#include <scil-util.h>

int main(int argc, char ** argv){
  if (argc == 1 || strcmp(argv[1], "-h") == 0 ){
    printf("Synopsis: %s <x> [y] [z]\n", argv[0]);
    exit(1);
  }
  int ret;
	scil_dims dims;

  switch(argc - 1){
    case (1):{
  	  scil_init_dims_1d(& dims, atol(argv[1]));
      break;
    }case (2):{
      scil_init_dims_2d(& dims, atol(argv[1]), atol(argv[2]));
      break;
    }case (3):{
      scil_init_dims_3d(& dims, atol(argv[1]), atol(argv[2]), atol(argv[3]));
      break;
    }default:{
      printf("Error can only plot up to 3D\n");
      exit(1);
    }
  }

	double * buffer_in = (double*) malloc(scil_get_data_size(SCIL_TYPE_DOUBLE, & dims));

  char * check_pattern = getenv("SCIL_PATTERN_TO_USE");

	for(int i=0; i < scilP_library_size(); i++){
		char * name = scilP_library_pattern_name(i);

    if( check_pattern != NULL && strcmp(name, check_pattern) != 0){
      printf("Skipping %s\n", name);
      continue;
    }

    char fullName[1024];
    sprintf(fullName, "%s.csv", name);

    printf("Processing %s\n", name);
		ret = scilP_library_create_pattern_double(i, & dims, buffer_in);
		assert( ret == SCIL_NO_ERR);
    scilU_plot(fullName, dims, buffer_in);
	}
  free(buffer_in);

  return 0;
}
