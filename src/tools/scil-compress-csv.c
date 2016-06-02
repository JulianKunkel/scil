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
#include <scil-option.h>

static int validate = 0;
static int verbose = 0;
static int compress = 0;
static int uncompress = 0;
static int cycle = 0;
static char * in_file = "";
static char * out_file = "";

static option_help known_args[] = {
  {'i', "in_file", "Input file (file format depends on mode)", OPTION_REQUIRED_ARGUMENT, 's', & in_file},
  {'o', "out_file", "Output file (file format depends on mode)", OPTION_REQUIRED_ARGUMENT, 's', & out_file},
  {'x', "decompress", "Infile is expected to be a binary compressed with this tool, outfile a CSV file",
    OPTION_FLAG, 'd', & uncompress},
  {'c', "compress", "Infile is expected to be a CSV file, outfile a binary",
    OPTION_FLAG, 'd' , & compress},
  {'C', NULL, "Cycle, for testing: Compress, then decompress and store the output. Files are CSV files",
      OPTION_FLAG, 'd' , & cycle},
  {'V', "validate", "Validate the output", OPTION_FLAG, 'd', & validate},
  {'v', "verbose", "Increase the verbosity level", OPTION_FLAG, 'd', & verbose},
  {0, 0, 0, 0, 0, NULL}
};


int main(int argc, char ** argv){
  scilO_parseOptions(argc, argv, known_args);



  return 0;
}
