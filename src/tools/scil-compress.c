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

#include <scil.h>
#include <scil-error.h>
#include <scil-option.h>
#include <scil-util.h>

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <plugins/file-plugin.h>

static int validate = 0;
static int verbose = 0;
static int compress = 0;
static int uncompress = 0;
static int cycle = 0;
static char * in_file = "";
static char * out_file = NULL;
static int compute_residual = 0;


static int use_max_value_as_fill_value = 0;
static int measure_time = 0;
static int print_hints = 0;

static char * in_file_format = "csv";
static char * out_file_format = "csv";

// data we process

static scil_dims_t dims;
static byte * input_data = NULL;
static byte * output_data = NULL;

static scil_file_plugin_t * in_plugin = NULL;
static scil_file_plugin_t * out_plugin = NULL;

int main(int argc, char ** argv){
  scil_context_t* ctx = NULL;
  scil_user_hints_t hints;
  scil_user_hints_t out_accuracy;

  double fake_abstol_value = 0;

  int ret;

  scilPr_initialize_user_hints(&hints);
  option_help known_args[] = {
    {'i', "in_file", "Input file (file format depends on mode)", OPTION_REQUIRED_ARGUMENT, 's', & in_file},
    {'o', "out_file", "Output file (file format depends on mode)", OPTION_OPTIONAL_ARGUMENT, 's', & out_file},
    {'I', "in_file_format", "Input file format, list shows all available formats", OPTION_OPTIONAL_ARGUMENT, 's', & in_file_format},
    {'O', "out_file_format", "Output file format", OPTION_OPTIONAL_ARGUMENT, 's', & out_file_format},
    {'x', "decompress", "Infile is expected to be a binary compressed with this tool, outfile a CSV file",OPTION_FLAG, 'd', & uncompress},
    {'c', "compress", "Infile is expected to be a CSV file, outfile a binary",OPTION_FLAG, 'd' , & compress},
    {'R', "residual", "(for decompression) compute/output the residual error instead of the data", OPTION_FLAG, 'd', & compute_residual},
    {'t', "time", "Measure time for the operation.", OPTION_FLAG, 'd', & measure_time},
    {'V', "validate", "Validate the output", OPTION_FLAG, 'd', & validate},
    {'v', "verbose", "Increase the verbosity level", OPTION_FLAG, 'd', & verbose},
    {'H', "print-hints", "Print the effective hints", OPTION_FLAG, 'd', & print_hints},
    {0, "use-max-value-as-fill", "Check for the maximum value and use it as fill value", OPTION_FLAG, 'd',  & use_max_value_as_fill_value},
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
    {0, "hint-lossless-range-up-to", NULL,  OPTION_OPTIONAL_ARGUMENT, 'F', & hints.lossless_data_range_up_to},
    {0, "hint-lossless-range-from", NULL,  OPTION_OPTIONAL_ARGUMENT, 'F', & hints.lossless_data_range_from},
    {0, "hint-fill-value", NULL,  OPTION_OPTIONAL_ARGUMENT, 'F', & hints.fill_value},
    {0, "hint-fake-absolute-tolerance-percent-max", "This is a fake hint. Actually it sets the abstol value based on the given percentage (enter 0.1 aka 10%% tolerance)",  OPTION_OPTIONAL_ARGUMENT, 'F', & fake_abstol_value},

    {0, "cycle", "For testing: Compress, then decompress and store the output. Files are CSV files",OPTION_FLAG, 'd' , & cycle},
    LAST_OPTION
  };

  int printhelp = 0;
  int parsed;
  SCIL_Datatype_t output_datatype;

  parsed = scilO_parseOptions(argc, argv, known_args, & printhelp);

  in_plugin = scil_find_plugin(in_file_format);
  if(! in_plugin){
    printf("Unknown format for input: %s\n", in_file_format);
    exit(1);
  }

  ret = scilO_parseOptions(argc-parsed, argv+parsed, in_plugin->get_options(), & printhelp);
  parsed += ret;

  out_plugin = scil_find_plugin(out_file_format);
  if(! out_plugin){
    printf("Unknown format for output: %s\n", out_file_format);
    exit(1);
  }
  ret = scilO_parseOptions(argc-parsed, argv+parsed, out_plugin->get_options(), & printhelp);
  if(printhelp != 0){
    printf("\nSynopsis: %s ", argv[0]);

    scilO_print_help(known_args, "-- <Input plugin options, see below> -- <Output plugin options, see below>\n");

    printf("\nPlugin options for input plugin %s\n", in_file_format);
    scilO_print_help(in_plugin->get_options(), "");

    printf("\nPlugin options for output plugin %s\n", out_file_format);
    scilO_print_help(out_plugin->get_options(), "");
    exit(0);
  }

  SCIL_Datatype_t input_datatype;
  size_t read_data_size;
  size_t array_size;

  scil_timer timer;
  scil_timer totalRun;
  double t_read = 0.0, t_write = 0.0, t_compress = 0.0, t_decompress = 0.0;
  scilU_start_timer(& totalRun);
  scilU_start_timer(& timer);
  ret = in_plugin->readData(in_file, & input_data, & input_datatype, & dims, & read_data_size);
  if (ret != 0){
    printf("The input file %s could not be read\n", in_file);
    exit(1);
  }
  t_read = scilU_stop_timer(timer);

  if (use_max_value_as_fill_value){
    double max, min;
    scilU_find_minimum_maximum(input_datatype, input_data, & dims, & min, & max);
    hints.fill_value = max;
  }

  if(verbose > 0){
    double max, min;
    scilU_find_minimum_maximum_with_excluded_points(input_datatype, input_data, & dims, & min, & max, hints.lossless_data_range_up_to,  hints.lossless_data_range_from, hints.fill_value);
    printf("Min: %.10e Max: %.10e\n", min, max);
  }

  array_size = scilPr_get_dims_size(& dims, input_datatype);


  if (fake_abstol_value > 0.0){
    double max, min;
    scilU_find_minimum_maximum_with_excluded_points(input_datatype, input_data, & dims, & min, & max, hints.lossless_data_range_up_to,  hints.lossless_data_range_from, hints.fill_value);
    if (min < 0 && max < -min){
	     max = -min;
    }

    double new_abs_tol = max * fake_abstol_value;
    if ( hints.absolute_tolerance > 0.0 ){
      printf("Error: don't set both the absolute_tolerance and the fake relative absolute tolerance!\n");
      exit(1);
    }

    printf("setting value %f %f %f\n", min, max, new_abs_tol);
    hints.absolute_tolerance = new_abs_tol;
  }

  ret = scilPr_create_context(&ctx, input_datatype, 0, NULL, &hints);
  assert(ret == SCIL_NO_ERR);

  if (print_hints){
    printf("Effective hints (only needed for compression)\n");
    scil_user_hints_t e = scilPr_get_effective_hints(ctx);
    scilPr_print_user_hints(& e);
  }


  size_t buff_size, input_size;

  input_size = scilPr_get_compressed_data_size_limit(&dims, input_datatype);
  output_data = (byte*) SAFE_MALLOC(input_size);

  if (cycle || (! compress && ! uncompress) ){
    printf("...compression and decompression\n");
    byte* result = (byte*) SAFE_MALLOC(input_size);

    scilU_start_timer(& timer);
    ret = scil_compress(result, input_size, input_data, & dims, & buff_size, ctx);
    t_compress = scilU_stop_timer(timer);
    assert(ret == SCIL_NO_ERR);

    byte* tmp_buff = (byte*) SAFE_MALLOC(input_size);
    scilU_start_timer(& timer);
    ret = scil_decompress(input_datatype, output_data, & dims, result, buff_size, tmp_buff);
    t_decompress = scilU_stop_timer(timer);
    assert(ret == SCIL_NO_ERR);

    free(tmp_buff);

    output_datatype = input_datatype;

    if (validate) {
        ret = scil_validate_compression(input_datatype, input_data, &dims, result, buff_size, ctx, &out_accuracy);
        if(ret != SCIL_NO_ERR){
          printf("SCIL validation error!\n");
        }
        if(print_hints){
          printf("Validation accuracy:");
          scilPr_print_user_hints(& out_accuracy);
        }
    }
    ret = scilPr_destroy_context(ctx);
    assert(ret == SCIL_NO_ERR);

  } else if (compress){
    printf("...compression\n");
    scilU_start_timer(& timer);
    ret = scil_compress(output_data, input_size, input_data, & dims, & buff_size, ctx);
    t_compress = scilU_stop_timer(timer);
    assert(ret == SCIL_NO_ERR);

    if (validate) {
        ret = scil_validate_compression(input_datatype, input_data, &dims, output_data, buff_size, ctx, &out_accuracy);
        if(ret != SCIL_NO_ERR){
          printf("SCIL validation error!\n");
        }
        if(print_hints){
          printf("Validation accuracy:");
          scilPr_print_user_hints(& out_accuracy);
        }
    }
    ret = scilPr_destroy_context(ctx);
    assert(ret == SCIL_NO_ERR);

    output_datatype = SCIL_TYPE_BINARY;
  } else if (uncompress){
    printf("...decompression\n");
    byte* tmp_buff = (byte*) SAFE_MALLOC(input_size);
    scilU_start_timer(& timer);
    ret = scil_decompress(input_datatype, output_data, & dims, input_data, read_data_size, tmp_buff);
    t_decompress = scilU_stop_timer(timer);
    free(tmp_buff);
    assert(ret == SCIL_NO_ERR);

    output_datatype = input_datatype;
  }

  // todo reformat into output format, if neccessary
  if (out_file != NULL){
    if ( compute_residual && (uncompress || cycle) ){
      // compute the residual error
      scilU_subtract_data(input_datatype, input_data, output_data, & dims);
    }

    scilU_start_timer(& timer);
    ret = out_plugin->writeData(out_file, output_data, output_datatype, buff_size, input_datatype, dims);
    t_write = scilU_stop_timer(timer);
    if (ret != 0){
      printf("The output file %s could not be written\n", out_file);
      exit(1);
    }
  }
	double runtime = scilU_stop_timer(totalRun);
  if(measure_time){
    printf("Size:\n");
    printf(" size, %ld\n size_compressed, %ld\n ratio, %f\n", array_size, buff_size, ((double) buff_size) / array_size);
    printf("Runtime:  %fs\n", runtime);
    printf(" read,       %fs, %f MiB/s\n", t_read, array_size/t_read/1024 /1024);
    if (t_compress > 0.0)
      printf(" compress,   %fs, %f MiB/s\n", t_compress, array_size/t_compress/1024 /1024);
    if (t_decompress > 0.0)
      printf(" decompress, %fs, %f MiB/s\n", t_decompress, array_size/t_decompress/1024 /1024);
    if (t_write > 0.0)
      printf(" write,      %fs, %f MiB/s\n", t_write, array_size/t_write/1024 /1024);
  }

  free(input_data);
  free(output_data);

  return 0;
}
