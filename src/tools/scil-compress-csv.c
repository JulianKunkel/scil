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

/*
 * This tool will compress / decompress a CSV
 */

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <scil.h>
#include <scil-util.h>
#include <scil-option.h>

static int validate = 0;
static int verbose = 0;
static int compress = 0;
static int uncompress = 0;
static int cycle = 0;
static char * in_file = "";
static char * out_file = "";
static char delim = ',';

static int data_type_float = 0;
static int measure_time = 0;
static int ignore_header = 0;
static int output_header = 0;
static int print_hints = 0;

// data we process

static scil_dims dims;
static int datatype = SCIL_TYPE_DOUBLE;
static byte * input_data = NULL;
static byte * output_data = NULL;

void readCSVData(){
  FILE * fd = fopen(in_file, "r");
  if (! fd){
    printf("Error file %s does not exist\n", in_file);
    exit(1);
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
      printf("Error reading file %s, number of columns varies, saw %d, expected %d\n", in_file, x, ref_x);
      exit(1);
    }
    ref_x = x;
    if (x > 1){
      y++;
    }
  }
  fclose(fd);

  printf("Read file %s: %d %d\n", in_file, x, y);

  if(y > 1){
    scil_init_dims_2d(& dims, x, y);
  }else{
    scil_init_dims_1d(& dims, x);
  }

  input_data = (byte*) malloc(scil_compress_buffer_size_bound(datatype, & dims));

  fd = fopen(in_file, "r");
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
}



void writeCSVData(){
  FILE * f = fopen(out_file, "w");
  if(f == NULL){
    printf("Could not open %s for write\n", out_file);
    exit(1);
  }
  double * buffer_in = (double *) output_data;
  if(dims.dims == 1){
    if (output_header)
      fprintf(f, "%zu\n", dims.length[0]);
    fprintf(f, "%.17f", buffer_in[0]);
    for(size_t x = 1; x < dims.length[0]; x++){
      fprintf(f, ",%.17f", buffer_in[x]);
    }
    fprintf(f, "\n");
  }else{
    if (output_header)
      fprintf(f, "%zu, %zu\n", dims.length[0], dims.length[1]);
    for(size_t y = 0; y < dims.length[1]; y++){
      fprintf(f, "%.17f", buffer_in[0+ y * dims.length[0]]);
      for(size_t x = 1; x < dims.length[0]; x++){
        fprintf(f, ",%.17f", buffer_in[x+ y * dims.length[0]]);
      }
      fprintf(f, "\n");
    }
  }
  fclose(f);

}

void readData(){
  FILE * f = fopen(in_file, "rb");

  if(f == NULL){
    printf("Could not open %s for read\n", in_file);
    exit(1);
  }

  uint8_t rows;
  size_t x ,y = 1, input_data_size, curr_pos;
  fread(&rows, sizeof(uint8_t), 1, f);
  fread(&x, sizeof(size_t), 1, f);

  if(rows > 1){
    fread(&y, sizeof(size_t), 1, f);
    scil_init_dims_2d(&dims, x, y);
  }else{
    scil_init_dims_1d(&dims, x);
  }

  curr_pos = ftell(f);
  fseek(f, 0L, SEEK_END);
  input_data_size = ftell(f) - curr_pos;
  fseek(f, curr_pos, SEEK_SET);

  input_data = (byte*) SAFE_MALLOC(input_data_size);

  fread(input_data, 1, input_data_size, f);

  fclose(f);
}

void writeData(){
  FILE * f = fopen(out_file, "wb");
  if(f == NULL){
    printf("Could not open %s for write\n", out_file);
    exit(1);
  }

  size_t buffer_in_size = dims.length[0] * sizeof(double);
  fwrite(&dims.dims, sizeof(dims.dims), 1, f);
  fwrite(&dims.length, sizeof(dims.length[0]), dims.dims, f);
  for (size_t i = 1; i < dims.dims; i++){
    buffer_in_size *= dims.length[i];
  }
  fwrite(output_data, 1, buffer_in_size, f);

  fclose(f);

}

int main(int argc, char ** argv){
  scil_context_p ctx;
  scil_hints hints;

  int ret;

  scil_init_hints(&hints);

  option_help known_args[] = {
    {'i', "in_file", "Input file (file format depends on mode)", OPTION_REQUIRED_ARGUMENT, 's', & in_file},
    {'o', "out_file", "Output file (file format depends on mode)", OPTION_REQUIRED_ARGUMENT, 's', & out_file},
    {'x', "decompress", "Infile is expected to be a binary compressed with this tool, outfile a CSV file",OPTION_FLAG, 'd', & uncompress},
    {'c', "compress", "Infile is expected to be a CSV file, outfile a binary",OPTION_FLAG, 'd' , & compress},
    {'f', "float", "Use float as datatype otherwise double.", OPTION_FLAG, 'd', & data_type_float},
    {'t', "time", "Measure time for the operation.", OPTION_FLAG, 'd', & measure_time},
    {'V', "validate", "Validate the output", OPTION_FLAG, 'd', & validate},
    {'v', "verbose", "Increase the verbosity level", OPTION_FLAG, 'd', & verbose},
    {'d', "delim", "Seperator", OPTION_OPTIONAL_ARGUMENT, 'c', & delim},
    {'H', "print-hints", "Print the effective hints", OPTION_FLAG, 'd', & print_hints},
    {0, "hint-force_compression_methods", NULL,  OPTION_OPTIONAL_ARGUMENT, 's', & hints.force_compression_methods},
    {0, "hint-absolute-tolerance", NULL,  OPTION_OPTIONAL_ARGUMENT, 'F', & hints.absolute_tolerance},
    {0, "hint-relative_tolerance_percent", NULL,  OPTION_OPTIONAL_ARGUMENT, 'F', & hints.relative_tolerance_percent},
    {0, "hint-relative_err_finest_abs_tolerance", NULL,  OPTION_OPTIONAL_ARGUMENT, 'F', & hints.relative_err_finest_abs_tolerance},
    {0, "hint-significant_bits", NULL,  OPTION_OPTIONAL_ARGUMENT, 'd', & hints.significant_bits},
    {0, "hint-significant_digits", NULL,  OPTION_OPTIONAL_ARGUMENT, 'd', & hints.significant_digits},
    {0, "hint-comp-speed-unit", NULL,  OPTION_OPTIONAL_ARGUMENT, 'e', & hints.comp_speed.unit},
    {0, "hint-decomp-speed-unit", NULL,  OPTION_OPTIONAL_ARGUMENT, 'e', & hints.decomp_speed.unit},
    {0, "hint-comp-speed", NULL,  OPTION_OPTIONAL_ARGUMENT, 'f', & hints.comp_speed.multiplier},
    {0, "hint-decomp-speed", NULL,  OPTION_OPTIONAL_ARGUMENT, 'f', & hints.decomp_speed.multiplier},

    {0, "add-output-header", "Provide an header for plotting", OPTION_FLAG, 'd', & output_header},
    {0, "ignore-header", "Ignore the header", OPTION_FLAG, 'd', & ignore_header},
    {0, "cycle", "For testing: Compress, then decompress and store the output. Files are CSV files",OPTION_FLAG, 'd' , & cycle},
    {0, 0, 0, 0, 0, NULL}
  };

  scilO_parseOptions(argc, argv, known_args);

  if (data_type_float){
    datatype = SCIL_TYPE_FLOAT;
  }

  ret = scil_create_compression_context(& ctx, datatype, &hints);
  assert(ret == SCIL_NO_ERR);

  if (print_hints){
    printf("Effective hints (only needed for compression)\n");
    scil_hints e = scil_retrieve_effective_hints(ctx);
    scil_hints_print(& e);
  }

  if (compress || cycle){
    readCSVData();
    printf("Read CSV data\n");
  }else{
    readData();
    printf("Read Binary data\n");
  }

  size_t buff_size, input_size;

  input_size = scil_compress_buffer_size_bound(datatype, & dims);
  output_data = (byte*) SAFE_MALLOC(input_size);

	scil_timer timer;
	scilU_start_timer(& timer);

  if (cycle){
    printf("...compression and decompression\n");
    byte* result = (byte*) SAFE_MALLOC(input_size);

    ret = scil_compress(result, input_size, (double*)input_data, & dims, & buff_size, ctx);
    assert(ret == SCIL_NO_ERR);
    ret = scil_destroy_compression_context(& ctx);
    assert(ret == SCIL_NO_ERR);

    byte* tmp_buff = (byte*) SAFE_MALLOC(buff_size);
    ret = scil_decompress(datatype, output_data, & dims, result, buff_size, tmp_buff);
    assert(ret == SCIL_NO_ERR);
    free(tmp_buff);

  } else if (compress){
    printf("...compression\n");
    ret = scil_compress(output_data, input_size, (double*)input_data, & dims, & buff_size, ctx);
    assert(ret == SCIL_NO_ERR);
    ret = scil_destroy_compression_context(& ctx);
    assert(ret == SCIL_NO_ERR);

  } else if (uncompress){
    printf("...decompression\n");
    byte* tmp_buff = (byte*) SAFE_MALLOC(input_size);
    ret = scil_decompress(datatype, output_data, & dims, input_data, input_size, tmp_buff);
    free(tmp_buff);
    assert(ret == SCIL_NO_ERR);
  }

	double runtime = scilU_stop_timer(timer);
  if(measure_time){
    printf("%fs \n", runtime);
  }

  if (uncompress || cycle){
    writeCSVData();
    printf("Write CSV data\n");
  }else{
    writeData();
    printf("Write Binary data\n");
  }

  return 0;
}
