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
#include <file-formats/file-brick-of-floats.h>

static int size_x = 500;
static int size_y = 500;
static int size_z = 100;
static int size_za = 0;
static int swap_order = 1;
static SCIL_Datatype_t datatype = SCIL_TYPE_FLOAT;

static option_help options [] = {
  {'x', NULL, "Dimension in X", OPTION_OPTIONAL_ARGUMENT, 'd', & size_x},
  {'y', NULL, "Dimension in Y", OPTION_OPTIONAL_ARGUMENT, 'd', & size_y},
  {'z', NULL, "Dimension in Z", OPTION_OPTIONAL_ARGUMENT, 'd', & size_z},
  {'a', NULL, "Dimension in Z'", OPTION_OPTIONAL_ARGUMENT, 'd', & size_za},
  {'S', "swap-endinaness", "Swap the byte order", OPTION_FLAG, 'd', & swap_order},
  LAST_OPTION
};

static option_help * get_options(){
  return options;
}


static int readData(const char * name, byte ** out_buf, SCIL_Datatype_t * out_datatype, scil_dims_t * out_dims, size_t * read_size){
  FILE * fd = fopen(name, "rb");
  if (! fd){
    return SCIL_EINVAL;
  }
  *out_datatype = datatype;
  scil_dims_initialize_4d(out_dims, size_x, size_y, size_z, size_za);
  const size_t data_size = scil_dims_get_size(out_dims, *out_datatype);

  byte * input_data = (byte*) malloc(scil_get_compressed_data_size_limit(out_dims, *out_datatype));
  //assert(size_y != 0 && size_z != 0 && size_za == 0 && size_x != 0);

  // index=x+dim_x×(y+dim_y×z)
  size_t ret = fread(input_data, data_size, 1, fd);
  if (ret != 1){
    printf("Error while reading data from %s\n", name);
    ret = SCIL_EINVAL;
  }else{
    ret = SCIL_NO_ERR;
  }
  fclose(fd);

  if (swap_order){
    // depending on the endianess, we may have to swap the endianess
    for(size_t p = 0; p < data_size; p += DATATYPE_LENGTH(datatype) ){
      for(int i=0; i < DATATYPE_LENGTH(datatype)/2; i++){
        byte tmp = input_data[p + i];
        input_data[p + i] = input_data[p + DATATYPE_LENGTH(datatype) - i - 1];
        input_data[p + DATATYPE_LENGTH(datatype) - i - 1] = tmp;
      }
    }
  }
  *out_buf = input_data;
  return ret;
}


static int writeData(const char * name, const byte * buf, SCIL_Datatype_t buf_datatype, size_t elements, SCIL_Datatype_t orig_datatype, scil_dims_t dims){
  FILE * fd = fopen(name, "wb");
  if (! fd){
    return SCIL_EINVAL;
  }
  assert(buf_datatype == orig_datatype);
  const size_t data_size = scil_dims_get_size(& dims, buf_datatype);
  size_t ret = fwrite(buf, data_size, 1, fd);
  if (ret != 1){
    printf("Error while writing data from %s\n", name);
    ret = SCIL_EINVAL;
  }else{
    ret = SCIL_NO_ERR;
  }

  fclose(fd);
  return 0;
}

scil_file_plugin_t brick_of_floats_plugin = {
  "bof",
  "bof",
  get_options,
  readData,
  writeData
};
