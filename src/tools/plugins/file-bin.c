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


static int readData(const char * name, byte ** out_buf, SCIL_Datatype_t * out_datatype, scil_dims_t * out_dims, size_t * read_size){
  FILE * f = fopen(name, "rb");

  if(f == NULL){
    printf("Could not open %s for read\n", name);
    return 1;
  }

  size_t input_data_size;
  size_t curr_pos;
  size_t expected_size;

  fread(out_datatype, 1, sizeof(SCIL_Datatype_t), f);
  fread(& expected_size, 1, sizeof(size_t), f);
  *read_size = expected_size;

  fread(out_dims, 1, sizeof(scil_dims_t), f);
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


static int writeData(const char * name, const byte * buf, SCIL_Datatype_t buf_datatype, size_t elements, SCIL_Datatype_t orig_datatype, scil_dims_t dims){
  FILE * f = fopen(name, "wb");
  if(f == NULL){
    printf("Could not open %s for write\n", name);
    return 1;
  }

  size_t buffer_in_size;
  if(buf_datatype == SCIL_TYPE_BINARY){
    buffer_in_size = elements;
  }else{
     buffer_in_size = scilPr_get_dims_size(& dims, buf_datatype);
  }

  fwrite(& orig_datatype, 1, sizeof(SCIL_Datatype_t), f);
  fwrite(& buffer_in_size, 1, sizeof(size_t), f);
  fwrite(& dims, 1, sizeof(scil_dims_t), f);
  fwrite(buf, 1, buffer_in_size, f);

  fclose(f);
  return 0;
}

scil_file_plugin_t bin_plugin = {
  "bin",
  get_options,
  readData,
  writeData
};
