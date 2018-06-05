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

#ifndef SCIL_PLUGIN_FILETYPE_H
#define SCIL_PLUGIN_FILETYPE_H

#include <scil-option.h>
#include <scil-dims.h>
#include <scil-datatypes.h>

typedef struct {
  char * name;
  char * extension;
  option_help *  (*get_options)();
  int (*readData)(const char * name, byte ** out_buf, SCIL_Datatype_t * out_datatype, scil_dims_t * out_dims, size_t * read_size);
  int (*writeData)(const char * name, const byte * buf, SCIL_Datatype_t buf_datatype, size_t elements, SCIL_Datatype_t orig_datatype, scil_dims_t dims);

  int (*openRead)(const char * name, SCIL_Datatype_t * out_datatype, scil_dims_t * out_dims, int * ncid, int * rh_id);
  int (*openWrite)(const char * name, SCIL_Datatype_t out_datatype, scil_dims_t out_dims, int * ncid, int * var_id);
  int (*readChunk)(const int ncid, SCIL_Datatype_t out_datatype, byte * buf, const int varid, const size_t * pos, const size_t * count);
  int (*writeChunk)(const int ncid, SCIL_Datatype_t out_datatype, const byte * buf, const int varid, const size_t * pos, const size_t * count);
  int (*closeFile)(const int ncid);
} scil_file_plugin_t;

scil_file_plugin_t * scil_find_plugin(const char * name);

#endif
