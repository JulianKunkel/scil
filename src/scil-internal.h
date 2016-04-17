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
  SCIL_COMPRESSOR_TYPE_DATATYPES,
  SCIL_COMPRESSOR_TYPE_DATATYPES_PRECONDITIONER
};

typedef struct{
    union{
        struct{
            int (*compress)(const scil_context* ctx, byte* restrict compressed_buf_in_out, size_t* restrict out_size, const byte*restrict data_in, const size_t in_size);
            int (*decompress)(byte*restrict data_out, const byte*restrict compressed_buf_in, const size_t in_size, size_t * uncomp_size_out);
        } Btype;

        struct{
          // for a preconditioner, we expect that the input buffer points only to the ND data, the output data contains
          // the header of the size as returned and then the preconditioned data.
            int (*compress_float)(const scil_context* ctx, float* restrict data_out, int * header_size_out, float*restrict data_in, const scil_dims_t dims);

          // it is the responsiblity of the decompressor to strip the header that is part of compressed_buf_in
            int (*decompress_float)(float*restrict data_inout, scil_dims_t dims, float*restrict compressed_buf_in);

            int (*compress_double)(const scil_context* ctx, double* restrict data_out, int * header_size_out, double*restrict data_in, const scil_dims_t dims);

            int (*decompress_double)(double*restrict data_inout, scil_dims_t dims, double*restrict compressed_buf_in);
        } DPrecond;

        struct{
            int (*compress_float)(const scil_context* ctx, byte* restrict compressed_buf_in_out, size_t* restrict out_size, float*restrict data_in, const scil_dims_t dims);

            int (*decompress_float)(float*restrict data_out, scil_dims_t dims, byte*restrict compressed_buf_in, const size_t in_size);

            int (*compress_double)(const scil_context* ctx, byte* restrict compressed_buf_in_out, size_t* restrict out_size, double*restrict data_in, const scil_dims_t dims);

            int (*decompress_double)( double*restrict data_out, scil_dims_t dims, byte*restrict compressed_buf_in, const size_t in_size);
        } DNtype;

        // TODO: Implement this
        struct{
          int i;
        } ICOtype;
    } c;


    const char * name;
    byte magic_number;

    enum compressor_type type;
} scil_compression_algorithm;

// at most we support chaining of 10 preconditioners
typedef struct {
  scil_compression_algorithm * pre_cond[10]; // preconditioners
  scil_compression_algorithm * data_compressor; // datatype compressor
  scil_compression_algorithm * byte_compressor; // byte compressor

  int size;
} scil_compression_chain_t;


struct scil_context_t{
  int lossless_compression_needed;
  scil_hints hints;

  // the last compressor used, could be used for debugging
  scil_compression_chain_t last_chain;
};



#endif
