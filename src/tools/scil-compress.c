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
static char * out_file = "";

static int measure_time = 0;
static int print_hints = 0;

static char * in_file_format = "csv";
static char * out_file_format = "csv";

// data we process

static scil_dims_t dims;
static SCIL_Datatype_t user_out_datatype = SCIL_TYPE_DOUBLE; // used for the intermediate output
static byte * input_data = NULL;
static byte * output_data = NULL;

static scil_file_plugin_t * in_plugin = NULL;
static scil_file_plugin_t * out_plugin = NULL;

int main(int argc, char ** argv){
  scil_context_t* ctx;
  scil_user_hints_t hints;
  scil_user_hints_t out_accuracy;

  int ret;

  scilPr_initialize_user_hints(&hints);

  option_help known_args[] = {
    {'i', "in_file", "Input file (file format depends on mode)", OPTION_REQUIRED_ARGUMENT, 's', & in_file},
    {'o', "out_file", "Output file (file format depends on mode)", OPTION_REQUIRED_ARGUMENT, 's', & out_file},
    {'I', "in_file_format", "Input file format, list shows all available formats", OPTION_OPTIONAL_ARGUMENT, 's', & in_file_format},
    {'O', "out_file_format", "Output file format", OPTION_OPTIONAL_ARGUMENT, 's', & out_file_format},
    {'x', "decompress", "Infile is expected to be a binary compressed with this tool, outfile a CSV file",OPTION_FLAG, 'd', & uncompress},
    {'c', "compress", "Infile is expected to be a CSV file, outfile a binary",OPTION_FLAG, 'd' , & compress},
    {'t', "time", "Measure time for the operation.", OPTION_FLAG, 'd', & measure_time},
    {'V', "validate", "Validate the output", OPTION_FLAG, 'd', & validate},
    {'v', "verbose", "Increase the verbosity level", OPTION_FLAG, 'd', & verbose},
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
  ret = scilO_parseOptions(argc - parsed, argv + parsed, in_plugin->get_options(), & printhelp);
  parsed += ret;

  out_plugin = scil_find_plugin(out_file_format);
  if(! out_plugin){
    printf("Unknown format for output: %s\n", out_file_format);
    exit(1);
  }
  ret = scilO_parseOptions(argc - parsed, argv + parsed, out_plugin->get_options(), & printhelp);

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

  ret = in_plugin->readData(in_file, & input_data, & input_datatype, & dims, & read_data_size);
  if (ret != 0){
    printf("The input file %s could not be read\n", in_file);
    exit(1);
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

	scil_timer timer;
	scilU_start_timer(& timer);

  if (cycle || (! compress && ! uncompress) ){
    printf("...compression and decompression\n");
    byte* result = (byte*) SAFE_MALLOC(input_size);

    ret = scil_compress(result, input_size, input_data, & dims, & buff_size, ctx);
    assert(ret == SCIL_NO_ERR);
    if (validate) {
        ret = scil_validate_compression(input_datatype, input_data, &dims, result, buff_size, ctx, &out_accuracy);
        assert(ret == SCIL_NO_ERR);
    }

    ret = scilPr_destroy_context(ctx);
    assert(ret == SCIL_NO_ERR);

    byte* tmp_buff = (byte*) SAFE_MALLOC(buff_size);
    ret = scil_decompress(input_datatype, output_data, & dims, result, buff_size, tmp_buff);
    assert(ret == SCIL_NO_ERR);

    free(tmp_buff);

    output_datatype = user_out_datatype;
  } else if (compress){
    printf("...compression\n");
    ret = scil_compress(output_data, input_size, input_data, & dims, & buff_size, ctx);
    assert(ret == SCIL_NO_ERR);

    if (validate) {
        ret = scil_validate_compression(input_datatype, input_data, &dims, output_data, buff_size, ctx, &out_accuracy);
        assert(ret == SCIL_NO_ERR);
    }
    ret = scilPr_destroy_context(ctx);
    assert(ret == SCIL_NO_ERR);

    output_datatype = SCIL_TYPE_BINARY;
  } else if (uncompress){
    printf("...decompression\n");
    byte* tmp_buff = (byte*) SAFE_MALLOC(input_size);
    ret = scil_decompress(input_datatype, output_data, & dims, input_data, read_data_size, tmp_buff);
    free(tmp_buff);
    assert(ret == SCIL_NO_ERR);

    output_datatype = user_out_datatype;
  }

	double runtime = scilU_stop_timer(timer);
  if(measure_time){
    printf("Runtime: %fs \n", runtime);
  }

  // todo reformat into output format, if neccessary
  ret = out_plugin->writeData(out_file, output_data, output_datatype, buff_size, input_datatype, dims);

  if (ret != 0){
    printf("The output file %s could not be written\n", out_file);
    exit(1);
  }

  free(input_data);
  free(output_data);

  return 0;
}
