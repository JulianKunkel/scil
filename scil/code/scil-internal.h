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

#ifndef SCIL_INTERNAL_HEADER_
#define SCIL_INTERNAL_HEADER_

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#include <scil.h>

enum compressor_type{
  SCIL_COMPRESSOR_TYPE_INDIVIDUAL_BYTES,
  SCIL_COMPRESSOR_TYPE_BASE_DATATYPE,
  SCIL_COMPRESSOR_TYPE_N_DIMENSIONAL,
  SCIL_COMPRESSOR_TYPE_ICOSAHEDRAL
};



typedef struct{
    union{
      struct{
        int (*compress)(const scil_context* ctx, byte* restrict compressed_buf_out, size_t* restrict out_size, const byte*restrict data_in, const size_t in_size);
        int (*decompress)(const scil_context* ctx, byte*restrict data_out, size_t*restrict out_size, const byte*restrict compressed_buf_in, const size_t in_size);
      } btype;

      struct{
        int (*compress)(const scil_context* ctx, byte* restrict compressed_buf_out, size_t* restrict out_size, const DataType*restrict data_in, const size_t in_size);
        int (*decompress)(const scil_context* ctx, DataType*restrict data_out, size_t*restrict out_size, const byte*restrict compressed_buf_in, const size_t in_size);
      } dtype;

      struct{ // where to put the dimensionality of the stream? Should we put it into the context_create?
        int (*compress)(const scil_context* ctx, byte* restrict compressed_buf_out, size_t* restrict out_size, const void*restrict nd_array, const size_t in_size);
        int (*decompress)(const scil_context* ctx, byte*restrict data_out, size_t*restrict out_size, const byte*restrict compressed_buf_in, const size_t in_size);
      } NDtype;

      struct{
        int (*compress)(const scil_context* ctx, byte* restrict compressed_buf_out, size_t* restrict out_size, const byte*restrict data_in, const size_t in_size);
        int (*decompress)(const scil_context* ctx, byte*restrict data_out, size_t*restrict out_size, const byte*restrict compressed_buf_in, const size_t in_size);
      } ICOtype;
    } c;
    const char * name;
    byte magic_number;

    // if true, we expect the compress function to compress individual bytes and takes count as bytes
    enum compressor_type type;
} scil_compression_algorithm;


struct scil_context_t{
  int lossless_compression_needed;
  scil_hints hints;

  // the last compressor used, could be used for debugging
  scil_compression_algorithm * last_algorithm;

  // bla bla
};


#endif
