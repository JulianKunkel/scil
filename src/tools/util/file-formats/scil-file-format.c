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

#include <string.h>
#include <stdio.h>

#include <file-formats/scil-file-format.h>

#include <file-formats/file-csv.h>
#include <file-formats/file-bin.h>
#include <file-formats/file-brick-of-floats.h>

#ifdef HAVE_NETCDF
#include <file-formats/file-netcdf.h>
#endif

static scil_file_plugin_t * file_plugins[] = {
& csv_plugin,
& bin_plugin,
& brick_of_floats_plugin,
#ifdef HAVE_NETCDF
& netcdf_plugin,
#endif
NULL
};

scil_file_plugin_t * scil_find_plugin(const char * name){
  int is_list = strcmp(name, "list") == 0;
  if (is_list){
    printf("Available plugins: ");
  }
  scil_file_plugin_t ** p_it = file_plugins;
  while(*p_it != NULL){
    if(is_list){
      printf("%s ", (*p_it)->name);
    }
    if((*p_it)->name == NULL){
      printf("Error, module \"%s\" not linked properly\n", name);
      return NULL;
    }
    if(strcmp((*p_it)->name, name) == 0) {
      // got it
      return *p_it;
    }
    p_it++;
  }
  return NULL;
}
