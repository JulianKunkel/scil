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
#include <stdio.h>
#include <assert.h>

#include <algo/precond-fp-delta.h>
#include <scil-error.h>

#define BLOCK_SIZE 100

// Repeat for each data type
#pragma GCC diagnostic ignored "-Wunused-parameter"
static int scil_delta_precond_compress_<DATATYPE>(const scil_context_t* ctx, <DATATYPE>* restrict data_out, byte*restrict header, int * header_size_out, <DATATYPE>*restrict data_in, const scil_dims_t* dims){
  const size_t size = scil_dims_get_count(dims);
  <DATATYPE>* din = (<DATATYPE>*) data_in;
  <DATATYPE>* dout = (<DATATYPE>*) data_out;
  const size_t repeats = size / BLOCK_SIZE;
  byte *restrict header_pos = header;
  size_t i = 0;
  for(size_t rep = 1; rep <= repeats; rep++){
    const size_t end = rep * BLOCK_SIZE;
    // find the minimum value
    <DATATYPE> minv = din[i];
    for( size_t t = i; t + 1 < end; t++){
      minv = din[t] < minv ? din[t] : minv;
    }
    for( ; i < end; i++){
      dout[i] = din[i] - minv;
    }
    memcpy(header_pos, & minv, sizeof(<DATATYPE>));
    //printf("C: %f %d\n", (double) minv, i);
    header_pos += sizeof(<DATATYPE>);
  }
  if (i < size){
    <DATATYPE> minv = din[i];
    for( size_t t = i; t + 1 < size; t++){
      minv = din[t] < minv ? din[t] : minv;
    }
    //printf("C: %f %d %d\n", (double) minv, i, size);
    for( ; i < size; i++){
      dout[i] = din[i] - minv;
    }
    memcpy(header_pos, & minv, sizeof(<DATATYPE>));
    header_pos += sizeof(<DATATYPE>);
  }
  *header_size_out = header_pos - header;
  //printf("CH: %d\n", *header_size_out);
  return SCIL_NO_ERR;
}

static int scil_delta_precond_decompress_<DATATYPE>(<DATATYPE>*restrict data_out, scil_dims_t* dims, <DATATYPE>*restrict data_in, byte*restrict header, int * header_parsed_out){
  const size_t size = scil_dims_get_count(dims);
  <DATATYPE>* din = (<DATATYPE>*) data_in;
  <DATATYPE>* dout = (<DATATYPE>*) data_out;
  const size_t repeats = size / BLOCK_SIZE;
  byte *restrict header_pos = header - (repeats + (size % BLOCK_SIZE == 0 ? 0 : 1)) * sizeof(<DATATYPE>) + 1;
  //printf("DH: %d %d %d %d\n", (repeats + (size % BLOCK_SIZE == 0 ? 0 : 1)) * sizeof(<DATATYPE>), size % BLOCK_SIZE, size, BLOCK_SIZE);

  size_t i = 0;
  for(size_t rep = 1; rep <= repeats; rep++){
    const size_t end = rep * BLOCK_SIZE;
    <DATATYPE> minv;
    memcpy(& minv, header_pos, sizeof(<DATATYPE>));
    header_pos += sizeof(<DATATYPE>);
    //printf("D: %f \n", (double) minv);
    for( ; i < end; i++){
      dout[i] = din[i] + minv;
    }
  }
  if (i < size){
    <DATATYPE> minv;
    memcpy(& minv, header_pos, sizeof(<DATATYPE>));
    header_pos += sizeof(<DATATYPE>);
    //printf("D: %f \n", (double) minv);
    for( ; i < size; i++){
      dout[i] = din[i] + minv;
    }
  }

  *header_parsed_out = header_pos - header;
  return SCIL_NO_ERR;
}

// End repeat


scilU_algorithm_t algo_precond_fp_delta = {
    .c.PFtype = {
        CREATE_INITIALIZER(scil_delta_precond)
    },
    "fpdelta",
    15,
    SCIL_COMPRESSOR_TYPE_DATATYPES_PRECONDITIONER_FIRST,
    0
};
