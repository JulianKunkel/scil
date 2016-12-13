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
#include <plugins/file-bin.h>

static option_help options [] = {
  LAST_OPTION
};

static option_help * get_options(){
  return options;
}


static int readData(const char * name, byte ** out_buf, SCIL_Datatype_t * out_datatype, scil_dims_t * out_dims){
  FILE * f = fopen(name, "rb");

  if(f == NULL){
    printf("Could not open %s for read\n", name);
    return 1;
  }

  uint8_t rows;
  size_t x ,y = 1, input_data_size, curr_pos;
  if(fread(&rows, sizeof(uint8_t), 1, f) == 0){
    printf("Could not read values from %s\n", name);
    return 1;
  }
  if(fread(&x, sizeof(size_t), 1, f) == 0){
    printf("Could not read values from %s\n", name);
    return 1;
  }

  if(rows > 1){
    if(fread(&y, sizeof(size_t), 1, f) == 0){
      printf("Could not read values from %s\n", name);
      return 1;
    }
    scilPr_initialize_dims_2d(out_dims, x, y);
  }else{
    scilPr_initialize_dims_1d(out_dims, x);
  }

  curr_pos = ftell(f);
  fseek(f, 0L, SEEK_END);
  input_data_size = ftell(f) - curr_pos;
  fseek(f, curr_pos, SEEK_SET);

  byte * input_data = (byte*) SAFE_MALLOC(input_data_size);

  if(fread(input_data, 1, input_data_size, f) == 0){
    printf("Could not read values from %s\n", name);
    return 1;
  }
  *out_buf = input_data;

  fclose(f);
  return 0;
}


static int writeData(const char * name, const byte * buf, SCIL_Datatype_t buf_datatype, SCIL_Datatype_t datatype, scil_dims_t dims){
  FILE * f = fopen(name, "wb");
  if(f == NULL){
    printf("Could not open %s for write\n", name);
    return 1;
  }
  size_t buffer_in_size = 0;
  if(datatype == SCIL_TYPE_FLOAT){
    buffer_in_size = dims.length[0] * sizeof(float);
  }else if(datatype == SCIL_TYPE_DOUBLE){
    buffer_in_size = dims.length[0] * sizeof(double);
  }else{
    printf("Unsupported datatype!\n");
    exit(1);
  }

  fwrite(&dims.dims, sizeof(dims.dims), 1, f);
  fwrite(&dims.length, sizeof(dims.length[0]), dims.dims, f);
  for (size_t i = 1; i < dims.dims; i++){
    buffer_in_size *= dims.length[i];
  }
  fwrite(buf, 1, buffer_in_size, f);

  fclose(f);
  return 0;
}

static int writeBinaryData(const char * name, const byte * buf,  size_t elements){
  return -1;
}

scil_file_plugin_t bin_plugin = {
  "bin",
  get_options,
  readData,
  writeData,
  writeBinaryData
};
