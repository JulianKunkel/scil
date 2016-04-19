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

// Error codes
#define SCIL_NO_ERR         0
#define SCIL_BUFFER_ERR     1
#define SCIL_MEMORY_ERR     2

extern const char* scil_error_messages[];

void scil_print_error(const uint8_t error_code);

#define SCIL_TYPE_double 1
#define SCIL_TYPE_float 0

// use sizeof(<DATATYPE>) in auto created code
static inline int datatype_length(enum SCIL_Datatype type){
  return type == SCIL_FLOAT ? sizeof(float) : sizeof(double);
}

enum compressor_type{
  SCIL_COMPRESSOR_TYPE_INDIVIDUAL_BYTES,
  SCIL_COMPRESSOR_TYPE_DATATYPES
};

typedef struct{
    union{
        struct{
            int (*compress)(const scil_context* ctx, byte* restrict compressed_buf_in_out, size_t* restrict out_size, const byte*restrict data_in, const size_t in_size);
            int (*decompress)(const scil_context* ctx, byte*restrict data_out, size_t exp_size, const byte*restrict compressed_buf_in, const size_t in_size);
        } Btype;

        struct{
            int (*compress_float)(const scil_context* ctx, byte* restrict compressed_buf_in_out,
              size_t* restrict out_size, float*restrict data_in, const scil_dims_t dims);

            int (*decompress_float)(const scil_context* ctx, float*restrict data_out,
              scil_dims_t dims, byte*restrict compressed_buf_in, const size_t in_size);

            int (*compress_double)(const scil_context* ctx, byte* restrict compressed_buf_in_out,
              size_t* restrict out_size, double*restrict data_in, const scil_dims_t dims);

            int (*decompress_double)(const scil_context* ctx, double*restrict data_out,
              scil_dims_t dims, byte*restrict compressed_buf_in, const size_t in_size);
        } DNtype;

        // TODO: Implement this
        struct{
          int i;
        } ICOtype;
    } c;
    const char * name;
    byte magic_number;

    // if true, we expect the compress function to compress individual bytes and takes count as bytes
    enum compressor_type type;
} scil_compression_algorithm;

// at most we support chaining of 10 compression algorithms
typedef struct {
  scil_compression_algorithm * algo[10];
  int size;
} scil_compression_chain_t;


struct scil_context_t{
  int lossless_compression_needed;
  scil_hints hints;

  // the last compressor used, could be used for debugging
  scil_compression_chain_t last_chain;
};

int scil_convert_significant_decimals_to_bits(int decimals);
int scil_convert_significant_bits_to_decimals(int bits);

#define MANTISSA_MAX_LENGTH 52

#define MANTISSA_LENGTH_float 23

typedef union {
  struct {
    uint32_t mantissa  : MANTISSA_LENGTH_float;
    uint32_t exponent : 8;
    uint32_t sign     : 1;
  } p;
	float f;
} datatype_cast_float;

#define MANTISSA_LENGTH_double 52

typedef union {
  struct {
    uint64_t mantissa  : MANTISSA_LENGTH_double;
    uint32_t exponent : 11;
    uint32_t sign     : 1;
  } p;
	double f;
} datatype_cast_double;


#endif
