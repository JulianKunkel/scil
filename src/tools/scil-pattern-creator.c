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

#include <scil-error.h>
#include <scil-patterns.h>
#include <scil-util.h>
#include <scil-option.h>
#include <file-formats/scil-file-format.h>

int main(int argc, char ** argv){
  int ret;
	scil_dims_t dims;
  int parsed;
  int x = 10, y = 0, z = 0, w = 0;
  int seed = 0;
  int printhelp = 0;
  char * check_pattern = "all";
  char * datatype_str = "double";
  SCIL_Datatype_t datatype;
  char * out_file_format = "csv";
  scil_file_plugin_t * out_plugin = NULL;

  option_help known_args[] = {
    {'x', NULL, "Cardinality in X direction", OPTION_REQUIRED_ARGUMENT, 'd', & x},
    {'y', NULL, "Cardinality in Y direction", OPTION_OPTIONAL_ARGUMENT, 'd', & y},
    {'z', NULL, "Cardinality in Z direction", OPTION_OPTIONAL_ARGUMENT, 'd', & z},
    {'w', NULL, "Cardinality in W direction", OPTION_OPTIONAL_ARGUMENT, 'd', & w},
    {'s', "seed", "The random seed", OPTION_OPTIONAL_ARGUMENT, 'd', & seed},
    {'p', "pattern", "The pattern to use (or all)", OPTION_OPTIONAL_ARGUMENT, 's', & check_pattern },
    {'D', "datatype", "The datatype to use", OPTION_OPTIONAL_ARGUMENT, 's', & datatype_str},
    {'O', "out_file_format", "Output file format", OPTION_OPTIONAL_ARGUMENT, 's', & out_file_format},
    LAST_OPTION
  };

  parsed = scilO_parseOptions(argc, argv, known_args, & printhelp);
  out_plugin = scil_find_plugin(out_file_format);
  if(! out_plugin){
    printf("Unknown format for output: %s\n", out_file_format);
    exit(1);
  }
  ret = scilO_parseOptions(argc - parsed, argv + parsed, out_plugin->get_options(), & printhelp);

  if(printhelp != 0){
    printf("\nSynopsis: %s ", argv[0]);

    scilO_print_help(known_args, "-- <Output plugin options, see below>\n");
    printf("\nPlugin options for output plugin %s\n", out_file_format);
    scilO_print_help(out_plugin->get_options(), "");
    exit(0);
  }
  datatype = scil_str_to_datatype(datatype_str);

  if(datatype == SCIL_TYPE_UNKNOWN){
    printf("Undefined datatype: %s\n", datatype_str);
    printf("Supported datatypes:");
    int i=0;
    const char * typ = scil_datatype_to_str(0);
    while(typ != NULL){
      printf(" %s", typ);
      i++;
      typ = scil_datatype_to_str(i);
    }
    printf("\n");
    exit(1);
  }

  scilP_set_random_seed(seed);

  scil_dims_initialize_4d(& dims, x, y, z, w);

  size_t data_size = scil_dims_get_size(&dims, datatype);
  byte* buffer_in = (byte*) malloc(data_size);

  for(int i=0; i < scilP_get_pattern_library_size(); i++){
	  char* name = scilP_get_library_pattern_name(i);

    if( strcmp(check_pattern, "all") != 0 && strcmp(name, check_pattern) != 0){
      continue;
    }

    char fullName[1024];
    sprintf(fullName, "%s.%s", name, out_plugin->extension);

    printf("Processing %s\n", fullName);
  	ret = scilP_create_library_pattern(buffer_in, datatype, & dims, i);
  	assert( ret == SCIL_NO_ERR);

    ret = out_plugin->writeData(fullName, buffer_in, datatype, data_size, datatype, dims);

    if (ret != 0){
      printf("The output file %s could not be written\n", fullName);
      exit(1);
    }
  }
  free(buffer_in);

  return 0;
}
