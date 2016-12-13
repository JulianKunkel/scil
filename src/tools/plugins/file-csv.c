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

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <scil-util.h>
#include <plugins/file-csv.h>

static char delim = ',';
static int ignore_header = 0;
static int output_header = 0;
static int data_type_float = 0;

static option_help options [] = {
  {'d', "delim", "Seperator", OPTION_OPTIONAL_ARGUMENT, 'c', & delim},
  {0, "add-output-header", "Provide an header for plotting", OPTION_FLAG, 'd', & output_header},
  {0, "ignore-header", "Ignore the header", OPTION_FLAG, 'd', & ignore_header},
  {'f', "float", "Use float as datatype otherwise double.", OPTION_FLAG, 'd', & data_type_float},
  LAST_OPTION
};

static option_help * get_options(){
  return options;
}


static int readData(const char * name, byte ** out_buf, SCIL_Datatype_t * out_datatype, scil_dims_t * out_dims){
  FILE * fd = fopen(name, "r");
  if (! fd){
    return -1;
  }

  if (data_type_float){
    *out_datatype = SCIL_TYPE_FLOAT;
  }else{
    *out_datatype = SCIL_TYPE_DOUBLE;
  }

  double dbl;
  int x = 0;
  int y = 0;
  int ref_x = -1;

  char * line = NULL;
  size_t len = 0;
  ssize_t read;
  char delimeter[2];
  delimeter[0] = delim;
  delimeter[1] = 0;

  if (ignore_header){
    read = getline(&line, &len, fd);
  }

  while ((read = getline(&line, &len, fd)) != -1) {
    char * data = strtok(line, delimeter);
    x = 0;
    while( data != NULL ){
      // count the number of elements.
      sscanf(data, "%lf", & dbl);
      data = strtok(NULL, delimeter);
      x++;
    }
    if (ref_x != -1 && x != ref_x && x > 1){
      printf("Error reading file %s, number of columns varies, saw %d, expected %d\n", name, x, ref_x);
      return -1;
    }
    ref_x = x;
    if (x > 1){
      y++;
    }
  }
  fclose(fd);

  printf("Read file %s: %d %d\n", name, x, y);

  if(y > 1){
    scilPr_initialize_dims_2d(out_dims, x, y);
  }else{
    scilPr_initialize_dims_1d(out_dims, x);
  }

  byte * input_data = (byte*) malloc(scilPr_get_compressed_data_size_limit(out_dims, *out_datatype));

  fd = fopen(name, "r");
  if (ignore_header){
    read = getline(&line, &len, fd);
  }

  size_t pos = 0;
  while ((read = getline(&line, &len, fd)) != -1) {
    char * data = strtok(line, delimeter);
    x = 0;
    while( data != NULL ){
      // count the number of elements.
      sscanf(data, "%lf", & dbl);
      if(data_type_float){
        ((float*) input_data)[pos] = (float) dbl;
      }else{
        ((double*) input_data)[pos] = dbl;
      }
      pos++;

      data = strtok(NULL, delimeter);
      x++;
    }
    ref_x = x;
    if (x > 1){
      y++;
    }
  }

  fclose(fd);

  *out_buf = input_data;
  return 0;
}



static int writeData(const char * name, const byte * buf, SCIL_Datatype_t buf_datatype, SCIL_Datatype_t datatype, scil_dims_t dims){
  FILE * f = fopen(name, "w");
  if(f == NULL){
    return -1;
  }
  double * buffer_in = (double*) buf;
  if(dims.dims == 1){
    if (output_header)
      fprintf(f, "%zu\n", dims.length[0]);
    fprintf(f, "%.17f", (double)buffer_in[0]);
    for(size_t x = 1; x < dims.length[0]; x++){
      fprintf(f, ",%.17f", (double)buffer_in[x]);
    }
    fprintf(f, "\n");
  }else{
    if (output_header)
      fprintf(f, "%zu, %zu\n", dims.length[0], dims.length[1]);
    for(size_t y = 0; y < dims.length[1]; y++){
      fprintf(f, "%.17f", (double)buffer_in[0+ y * dims.length[0]]);
      for(size_t x = 1; x < dims.length[0]; x++){
        fprintf(f, ",%.17f", (double)buffer_in[x+ y * dims.length[0]]);
      }
      fprintf(f, "\n");
    }
  }
  fclose(f);
  return 0;
}

static int writeBinaryData(const char * name, const byte * buf, size_t elements){
  return -1;
}

scil_file_plugin_t csv_plugin = {
  "csv",
  get_options,
  readData,
  writeData,
  writeBinaryData
};
