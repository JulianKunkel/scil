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

#define SCIL_TYPE_double 1
#define SCIL_TYPE_float 0

// use sizeof(<DATATYPE>) in auto created code
static inline int datatype_length(enum SCIL_Datatype type){
  return type == SCIL_FLOAT ? sizeof(float) : sizeof(double);
}

enum compressor_type{
  SCIL_COMPRESSOR_TYPE_INDIVIDUAL_BYTES,
  SCIL_COMPRESSOR_TYPE_1D,
  SCIL_COMPRESSOR_TYPE_2D,
  SCIL_COMPRESSOR_TYPE_3D,
  SCIL_COMPRESSOR_TYPE_N_DIMENSIONAL,
  SCIL_COMPRESSOR_TYPE_ICOSAHEDRAL
};

typedef struct{
    union{
        struct{
            int (*compress)(const scil_context* ctx, byte* restrict compressed_buf_out, uint64_t* restrict out_size, const byte*restrict data_in, const uint64_t in_size);
            int (*decompress)(const scil_context* ctx, byte*restrict data_out, uint64_t exp_size, const byte*restrict compressed_buf_in, const uint64_t in_size);
        } Btype;

        struct{
            int (*compress_float)(const scil_context* ctx, byte* restrict compressed_buf_out,
              uint64_t* restrict out_size, const float*restrict data_in, const uint64_t in_size);

            int (*decompress_float)(const scil_context* ctx, float*restrict data_out,
              uint64_t exp_count, const byte*restrict compressed_buf_in, const uint64_t in_size);

            int (*compress_double)(const scil_context* ctx, byte* restrict compressed_buf_out,
              uint64_t* restrict out_size, const double*restrict data_in, const uint64_t in_size);

            int (*decompress_double)(const scil_context* ctx, double*restrict data_out,
              uint64_t exp_count, const byte*restrict compressed_buf_in, const uint64_t in_size);
        } D1type;

        // TODO: fix parameter types
        struct{ // where to put the dimensionality of the stream? Should we put it into the context_create?
          int i;
        } D2type;

        // TODO: fix parameter types
        struct{ // where to put the dimensionality of the stream? Should we put it into the context_create?
          int i;
        } D3type;

        // TODO: fix parameter types
        struct{ // where to put the dimensionality of the stream? Should we put it into the context_create?
          int i;
        } DNtype;

        // TODO: fix parameter types
        struct{
          int i;
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

int scil_convert_significant_decimals_to_bits(int decimals);
int scil_convert_significant_bits_to_decimals(int bits);

#define MANTISSA_MAX_LENGTH 52


#define MANTISA_LENGTH_float 23

typedef union {
  struct {
    uint32_t mantisa  : MANTISA_LENGTH_float;
    uint32_t exponent : 8;
    uint32_t sign     : 1;
  } p;
	float f;
} datatype_cast_float;

#define MANTISA_LENGTH_double 52

typedef union {
  struct {
    uint64_t mantisa  : MANTISA_LENGTH_double;
    uint32_t exponent : 11;
    uint32_t sign     : 1;
  } p;
	double f;
} datatype_cast_double;


#endif
