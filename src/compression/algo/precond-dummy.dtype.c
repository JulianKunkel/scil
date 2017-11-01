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

//Supported datatypes: float double int8_t int16_t int32_t int64_t

#include <algo/precond-dummy.h>

#include <scil-error.h>
#include <scil-util.h>

#include <string.h>


// Repeat for each data type
#pragma GCC diagnostic ignored "-Wunused-parameter"
static int scil_dummy_precond_compress_<DATATYPE>(const scil_context_t* ctx, <DATATYPE>* restrict data_out, byte*restrict header, int * header_size_out, <DATATYPE>*restrict data_in, const scil_dims_t* dims){
  size_t size = scil_dims_get_size(dims, SCIL_TYPE_<DATATYPE_UPPER>);
  memcpy(data_out, data_in, size);
  memcpy(header, "DUMMY", 5);
  *header_size_out = 5;
  return SCIL_NO_ERR;
}

static int scil_dummy_precond_decompress_<DATATYPE>(<DATATYPE>*restrict data_out, scil_dims_t* dims, <DATATYPE>*restrict compressed_buf_in, byte*restrict header, int * header_parsed_out){
  size_t size = scil_dims_get_size(dims, SCIL_TYPE_<DATATYPE_UPPER>);
  memcpy(data_out, compressed_buf_in, size);
  if (memcmp(header-4, "DUMMY", 5) != 0){
    return SCIL_BUFFER_ERR;
  }
  *header_parsed_out = 5;
  return SCIL_NO_ERR;
}

// End repeat


scilU_algorithm_t algo_precond_dummy = {
    .c.PFtype = {
        CREATE_INITIALIZER(scil_dummy_precond)
    },
    "dummy-precond",
    8,
    SCIL_COMPRESSOR_TYPE_DATATYPES_PRECONDITIONER_FIRST,
    0
};
