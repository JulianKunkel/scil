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
#include <scil-util.h>

#ifdef DEBUG
  #define debug(...) fprintf(stderr, "[SCIL DEBUG] "__VA_ARGS__);
#else
  #define debug(...)
#endif

#define critical(...) { fprintf(stderr, "[SCIL CRITICAL] "__VA_ARGS__); exit(1); }
#define warn(...) fprintf(stderr, "[SCIL WARN] "__VA_ARGS__);

#define FUNC_START debug("CALL %s\n", __PRETTY_FUNCTION__);

#define PRECONDITIONER_LIMIT 10

enum compressor_type{
  SCIL_COMPRESSOR_TYPE_INDIVIDUAL_BYTES,
  SCIL_COMPRESSOR_TYPE_DATATYPES,
  SCIL_COMPRESSOR_TYPE_DATATYPES_PRECONDITIONER
};

/*
 An algorithm implementation can be sure that the compression output buffer is at least 2x the size of the input data.
 */
typedef struct{
    union{
        struct{
            int (*compress)(const scil_context_p ctx, byte* restrict compressed_buf_in_out, size_t* restrict out_size, const byte*restrict data_in, const size_t in_size);
            int (*decompress)(byte*restrict data_out, size_t buff_out_size,  const byte*restrict compressed_buf_in, const size_t in_size, size_t * uncomp_size_out);
        } Btype;

        struct{
          // for a preconditioner, we expect that the input buffer points only to the ND data, the output data contains
          // the header of the size as returned and then the preconditioned data.
            int (*compress_float)(const scil_context_p ctx, float* restrict data_out, int * header_size_out, float*restrict data_in, const scil_dims* dims);

          // it is the responsiblity of the decompressor to strip the header that is part of compressed_buf_in
            int (*decompress_float)(float*restrict data_inout, scil_dims*const dims, float*restrict compressed_buf_in);

            int (*compress_double)(const scil_context_p ctx, double* restrict data_out, int * header_size_out, double*restrict data_in, const scil_dims* dims);

            int (*decompress_double)(double*restrict data_inout, scil_dims*const dims, double*restrict compressed_buf_in);
        } Dtype;

        struct{
            int (*compress_float)(const scil_context_p ctx, byte* restrict compressed_buf_in_out, size_t* restrict out_size, float*restrict data_in, const scil_dims* dims);

            int (*decompress_float)(float*restrict data_out, scil_dims* dims, byte*restrict compressed_buf_in, const size_t in_size);

            int (*compress_double)(const scil_context_p ctx, byte* restrict compressed_buf_in_out, size_t* restrict out_size, double*restrict data_in, const scil_dims* dims);

            int (*decompress_double)( double*restrict data_out, scil_dims* dims, byte*restrict compressed_buf_in, const size_t in_size);
        } DNtype;

        // TODO: Implement this
        struct{
          int i;
        } ICOtype;
    } c;


    const char * name;
    byte magic_number;

    enum compressor_type type;
    char is_lossy; // byte compressors are expected to be lossless anyway
} scil_compression_algorithm;

// at most we support chaining of 10 preconditioners
typedef struct {
  scil_compression_algorithm * pre_cond[PRECONDITIONER_LIMIT]; // preconditioners
  scil_compression_algorithm * data_compressor; // datatype compressor
  scil_compression_algorithm * byte_compressor; // byte compressor

  char precond_count;
  char total_size; // includes data and byte compressors
  char is_lossy;
} scil_compression_chain_t;


struct scil_context_t{
  int lossless_compression_needed;
  enum SCIL_Datatype datatype;
  scil_hints hints;

  // the last compressor used, could be used for debugging
  scil_compression_chain_t chain;
};

int scilI_parse_compression_algorithms(scil_compression_chain_t * chain, char * str_in);
size_t scilI_get_data_size(enum SCIL_Datatype type, const scil_dims* dims);
scil_compression_algorithm * scilI_find_compressor_by_name(const char * name);

#endif
