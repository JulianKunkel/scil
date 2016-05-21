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

void plot1D(const char* name, scil_dims dims, double * buffer_in){
  FILE * f = fopen(name, "w");
  fprintf(f, "%zu\n", dims.length[0]);
  fprintf(f, "%f", buffer_in[0]);
  for(size_t x = 1; x < dims.length[0]; x++){
    fprintf(f, ",%f", buffer_in[x]);
  }
  fprintf(f, "\n");
  fclose(f);
}

void plot2D(const char* name, scil_dims dims, double * buffer_in){
  FILE * f = fopen(name, "w");

  fprintf(f, "%zu, %zu\n", dims.length[0], dims.length[1]);
  for(size_t y = 0; y < dims.length[1]; y++){
    fprintf(f, "%f", buffer_in[0+ y * dims.length[0]]);
    for(size_t x = 1; x < dims.length[0]; x++){
      fprintf(f, ",%f", buffer_in[x+ y * dims.length[0]]);
    }
    fprintf(f, "\n");
  }
  fclose(f);
}

void plot3D(const char* name, scil_dims dims, double * buffer_in){
  FILE * f = fopen(name, "w");
  fprintf(f, "%zu, %zu, %zu\n", dims.length[0], dims.length[1], dims.length[2]);

  for(size_t z = 0; z < dims.length[2]; z++){
    size_t z_ = z * dims.length[0]*dims.length[1];
    for(size_t y = 0; y < dims.length[1]; y++){
      fprintf(f, "%f", buffer_in[0+ y * dims.length[0] + z_]);
      for(size_t x = 1; x < dims.length[0]; x++){
        fprintf(f, ",%f", buffer_in[x+ y * dims.length[0] + z_]);
      }
      fprintf(f, "\n");
    }
  }
  fclose(f);
}

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

	for(int i=0; i < scilP_library_size(); i++){
		char * name = scilP_library_pattern_name(i);
    char fullName[1024];
    sprintf(fullName, "%s.csv", name);

    printf("Processing %s\n", name);
		ret = scilP_library_create_pattern_double(i, & dims, buffer_in);
		assert( ret == SCIL_NO_ERR);
    switch(argc - 1){
      case (1):{
  	     plot1D(fullName, dims, buffer_in);
         break;
       }case (2):{
         plot2D(fullName, dims, buffer_in);
         break;
       }case (3):{
         plot3D(fullName, dims, buffer_in);
         break;
       }default:{
         printf("Error can only plot up to 3D\n");
         exit(1);
       }
     }

	}
  free(buffer_in);

  return 0;
}
