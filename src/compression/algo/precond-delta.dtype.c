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

#include <string.h>
#include <assert.h>

#include <algo/precond-delta.h>
#include <scil-error.h>


#include <stdio.h>

// Repeat for each data type
#pragma GCC diagnostic ignored "-Wunused-parameter"
int scil_delta_precond_compress_<DATATYPE>(const scil_context_t* ctx, <DATATYPE>* restrict data_out, byte*restrict header, int * header_size_out, <DATATYPE>*restrict data_in, const scil_dims_t* dims){
  const size_t size = scil_dims_get_count(dims);
  switch(sizeof(<DATATYPE>)){
    case 8: {
      uint64_t* din = (uint64_t*) data_in;
      uint64_t* dout = (uint64_t*) data_out;
      dout[0] = din[0];
      for(size_t i=1; i < size; i++){
        dout[i] = din[i] - din[i-1];
      }
      break;
    }
    case 4: {
      uint32_t* din = (uint32_t*) data_in;
      uint32_t* dout = (uint32_t*) data_out;
      dout[0] = din[0];
      for(size_t i=1; i < size; i++){
        dout[i] = din[i] - din[i-1];
      }
      break;
    }
    case 2: {
      uint16_t* din = (uint16_t*) data_in;
      uint16_t* dout = (uint16_t*) data_out;
      dout[0] = din[0];
      for(size_t i=1; i < size; i++){
        dout[i] = din[i] - din[i-1];
      }
      break;
    }
    case 1: {
      uint8_t* din = (uint8_t*) data_in;
      uint8_t* dout = (uint8_t*) data_out;
      dout[0] = din[0];
      for(size_t i=1; i < size; i++){
        dout[i] = din[i] - din[i-1];
      }
      break;
    }
  }
  *header_size_out = 0;
  return SCIL_NO_ERR;
}

int scil_delta_precond_decompress_<DATATYPE>(<DATATYPE>*restrict data_out, scil_dims_t* dims, <DATATYPE>*restrict data_in, byte*restrict header, int * header_parsed_out){
  const size_t size = scil_dims_get_count(dims);
  switch(sizeof(<DATATYPE>)){
    case 8: {
      uint64_t* din = (uint64_t*) data_in;
      uint64_t* dout = (uint64_t*) data_out;
      dout[0] = din[0];
      for(size_t i=1; i < size; i++){
        dout[i] = din[i] + dout[i-1];
      }
      break;
    }
    case 4: {
      uint32_t* din = (uint32_t*) data_in;
      uint32_t* dout = (uint32_t*) data_out;
      dout[0] = din[0];
      for(size_t i=1; i < size; i++){
        dout[i] = din[i] + dout[i-1];
      }
      break;
    }
    case 2: {
      uint16_t* din = (uint16_t*) data_in;
      uint16_t* dout = (uint16_t*) data_out;
      dout[0] = din[0];
      for(size_t i=1; i < size; i++){
        dout[i] = din[i] + dout[i-1];
      }
      break;
    }
    case 1: {
      uint8_t* din = (uint8_t*) data_in;
      uint8_t* dout = (uint8_t*) data_out;
      dout[0] = din[0];
      for(size_t i=1; i < size; i++){
        dout[i] = din[i] + dout[i-1];
      }
      break;
    }
  }
  *header_parsed_out = 0;
  return SCIL_NO_ERR;
}

// End repeat


scilU_algorithm_t algo_precond_delta = {
    .c.PFtype = {
        CREATE_INITIALIZER(scil_delta_precond)
    },
    "delta",
    14,
    SCIL_COMPRESSOR_TYPE_DATATYPES_PRECONDITIONER_FIRST,
    0
};
