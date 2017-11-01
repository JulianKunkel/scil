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
#include <file-formats/file-csv.h>

static char delim = ',';
static int ignore_header = 0;
static int data_type_float = 0;
static char comment_char = '#';
static char scientific = 0;

static option_help options [] = {
  {'d', "delim", "Separator", OPTION_OPTIONAL_ARGUMENT, 'c', & delim},
  {0, "ignore-header", "Ignore the header/do not write it", OPTION_FLAG, 'd', & ignore_header},
  {'f', "float", "Use float as datatype for data without header, otherwise double.", OPTION_FLAG, 'd', & data_type_float},
  {'C', "comment-delim", "Characters used in the beginning of a line indicating a comment", OPTION_OPTIONAL_ARGUMENT, 'c', & comment_char},
  {'S', "scientific", "Process scientific notation.", OPTION_FLAG, 'd', & scientific},
  LAST_OPTION
};

static option_help * get_options(){
  return options;
}


static int readData(const char * name, byte ** out_buf, SCIL_Datatype_t * out_datatype, scil_dims_t * out_dims, size_t * read_size){
  FILE * fd = fopen(name, "r");
  if (! fd){
    return -1;
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

  int firstline = 1;
  int foundHeader = 0;
  while ((read = getline(&line, &len, fd)) != -1) {
    if (firstline){
      firstline = 0;
      if (line[0] == comment_char){
        // this is the header
        if (! ignore_header){
          //printf("Header: %s\n", line);
          int dim_count;
          size_t dims[4];
          char pattern[256];
          sprintf(pattern, "# %%d%c%%d%c%%zu%c%%zu%c%%zu%c%%zu", delim, delim,delim,delim,delim);
          int ret = sscanf(line, pattern, out_datatype, & dim_count, & dims[0], &dims[1], & dims[2], &dims[3]);
          if( ret != 6){
            printf("Invalid header!\n");
            continue;
          }
          scil_dims_initialize_4d(out_dims, dims[0], dims[1], dims[2], dims[3]);
          foundHeader = 1;
          break;
        }
      }else if(! ignore_header){
        printf("Warning: no header found in file\n");
      }
    }
    if (line[0] == comment_char){
      continue;
    }

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

  if (! foundHeader){
    printf("Read file %s: %d %d\n", name, x, y);

    if (data_type_float){
      *out_datatype = SCIL_TYPE_FLOAT;
    }else{
      *out_datatype = SCIL_TYPE_DOUBLE;
    }

    if(y > 1){
      scil_dims_initialize_2d(out_dims, x, y);
    }else{
      scil_dims_initialize_1d(out_dims, x);
    }
  }
  byte * input_data = (byte*) malloc(scil_get_compressed_data_size_limit(out_dims, *out_datatype));


  fd = fopen(name, "r");
  size_t pos = 0;
  while ((read = getline(&line, &len, fd)) != -1) {
    if (line[0] == comment_char){
      continue;
    }
    char * data = strtok(line, delimeter);
    x = 0;
    while( data != NULL ){
      // count the number of elements.
      if (scientific){
        sscanf(data, "%le", & dbl);
      }else{
        sscanf(data, "%lf", & dbl);
      }
      switch(*out_datatype){
        case(SCIL_TYPE_DOUBLE):
          ((double*) input_data)[pos] = dbl;
          break;
        case(SCIL_TYPE_FLOAT):
          ((float*) input_data)[pos] = (float) dbl;
          break;
        case(SCIL_TYPE_INT8):
          ((int8_t*) input_data)[pos] = (int8_t) dbl;
          break;
        case(SCIL_TYPE_INT16):
          ((int16_t*) input_data)[pos] = (int16_t) dbl;
          break;
        case(SCIL_TYPE_INT32):
          ((int32_t*) input_data)[pos] = (int32_t) dbl;
          break;
        case(SCIL_TYPE_INT64):
          ((int64_t*) input_data)[pos] = (int64_t) dbl;
          break;
        default:
          printf("Not supported in readData\n");
      }
      pos++;

      data = strtok(NULL, delimeter);
    }
  }

  fclose(fd);

  *out_buf = input_data;
  return 0;
}

static void printToFile(FILE * f, const byte * buf, size_t position,  SCIL_Datatype_t datatype){
  switch(datatype){
    case(SCIL_TYPE_DOUBLE):
      if(scientific){
        fprintf(f, "%.17e", ((double*) buf)[position]);
      }else{
        fprintf(f, "%.17f", ((double*) buf)[position]);
      }
      break;
    case(SCIL_TYPE_FLOAT):
      if(scientific){
        fprintf(f, "%.8e", (double) ((float*) buf)[position]);
      }else{
        fprintf(f, "%.8f", (double) ((float*) buf)[position]);
      }
      break;
    case(SCIL_TYPE_INT8):
      fprintf(f, "%d", (int8_t) ((int8_t*) buf)[position]);
      break;
    case(SCIL_TYPE_INT16):
      fprintf(f, "%d", (int16_t) ((int16_t*) buf)[position]);
      break;
    case(SCIL_TYPE_INT32):
      fprintf(f, "%d", (int32_t) ((int32_t*) buf)[position]);
      break;
    case(SCIL_TYPE_INT64):
      fprintf(f, "%ld", (int64_t) ((int64_t*) buf)[position]);
      break;
    default:
      printf("Not supported in writeData\n");
  }
}

static int writeData(const char * name, const byte * buf, SCIL_Datatype_t buf_datatype, size_t elements, SCIL_Datatype_t orig_datatype, scil_dims_t dims){
  FILE * f = fopen(name, "w");
  if(f == NULL){
    return -1;
  }

  if (! ignore_header){
    fprintf(f, "# %d%c%d%c", orig_datatype, delim, dims.dims, delim);
    for(int i=0; i < SCIL_DIMS_MAX; i++) {
      fprintf(f, "%zu%c", dims.length[i], delim);
    }
    fprintf(f, "\n");
  }
  if(dims.dims == 1){
    printToFile(f, buf, 0, buf_datatype);
    for(size_t x = 1; x < dims.length[0]; x+=1){
      fprintf(f, "%c", delim);
      printToFile(f, buf, x, buf_datatype);
    }
    fprintf(f, "\n");
  }else if(dims.dims == 2){
    for(size_t y = 0; y < dims.length[1]; y+=1){
      printToFile(f, buf, y * dims.length[0], buf_datatype);
      for(size_t x = 1; x < dims.length[0]; x+=1){
        fprintf(f, "%c", delim);
        printToFile(f, buf, x+ y * dims.length[0], buf_datatype);
      }
      fprintf(f, "\n");
    }
  }else if(dims.dims == 3){
    for(size_t z = 0; z < dims.length[2]; z+=1){
      for(size_t y = 0; y < dims.length[1]; y+=1){
        size_t offset = (y + z * dims.length[1]) * dims.length[0];
        printToFile(f, buf, offset, buf_datatype);
        for(size_t x = 1; x < dims.length[0]; x+=1){
          fprintf(f, "%c", delim);
          printToFile(f, buf, x + offset, buf_datatype);
        }
        fprintf(f, "\n");
      }
    }
  }else if(dims.dims == 4){
    for(size_t w = 0; w < dims.length[3]; w+=1){
      for(size_t z = 0; z < dims.length[2]; z+=1){
        for(size_t y = 0; y < dims.length[1]; y+=1){
          size_t offset = dims.length[0] * (y + dims.length[1] * (z + w * dims.length[2]));
          printToFile(f, buf, offset, buf_datatype);
          for(size_t x = 1; x < dims.length[0]; x+=1){
            fprintf(f, "%c", delim);
            printToFile(f, buf, x + offset, buf_datatype);
          }
          fprintf(f, "\n");
        }
      }
    }
  }else{
    printf("3Dims not supported in CSV writer\n");
    return SCIL_EINVAL;
  }
  fclose(f);
  return SCIL_NO_ERR;
}

scil_file_plugin_t csv_plugin = {
  "csv",
  "csv",
  get_options,
  readData,
  writeData
};
