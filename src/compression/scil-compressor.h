#ifndef SCIL_ALGOS_H
#define SCIL_ALGOS_H

#include <scil-context.h>

enum compressor_type{
  SCIL_COMPRESSOR_TYPE_DATATYPES_PRECONDITIONER_FIRST,
  SCIL_COMPRESSOR_TYPE_DATATYPES_CONVERTER,
  SCIL_COMPRESSOR_TYPE_DATATYPES_PRECONDITIONER_SECOND,
  SCIL_COMPRESSOR_TYPE_DATATYPES,
  SCIL_COMPRESSOR_TYPE_INDIVIDUAL_BYTES
};

/*
 An algorithm implementation can be sure that the compression output buffer is at least 2x the size of the input data.
 */
typedef struct scil_compression_algorithm {
  union{
    struct{
      // for a preconditioner first stage, we expect that the input buffer points only to the ND data, the output data contains
      // the header of the size as returned and then the preconditioned data.
      int (*compress_float)(const scil_context_t* ctx, float* restrict data_out, byte*restrict header, int * header_size_out, float*restrict data_in, const scil_dims_t* dims);
      // it is the responsiblity of the decompressor to strip the header that is part of compressed_buf_in
      int (*decompress_float)(float*restrict data_out, scil_dims_t* dims, float*restrict compressed_buf_in, byte*restrict header_end, int * header_parsed_out);

      int (*compress_double)(const scil_context_t* ctx, double* restrict data_out, byte*restrict header, int * header_size_out, double*restrict data_in, const scil_dims_t* dims);
      int (*decompress_double)(double*restrict data_out, scil_dims_t* dims, double*restrict compressed_buf_in, byte*restrict header_end, int * header_parsed_out);

      int (*compress_int8)(const scil_context_t* ctx, int8_t* restrict data_out, byte*restrict header, int * header_size_out, int8_t*restrict data_in, const scil_dims_t* dims);
      int (*decompress_int8)(int8_t*restrict data_out, scil_dims_t* dims, int8_t*restrict compressed_buf_in, byte*restrict header_end, int * header_parsed_out);

      int (*compress_int16)(const scil_context_t* ctx, int16_t* restrict data_out, byte*restrict header, int * header_size_out, int16_t*restrict data_in, const scil_dims_t* dims);
      int (*decompress_int16)(int16_t*restrict data_out, scil_dims_t* dims, int16_t*restrict compressed_buf_in, byte*restrict header_end, int * header_parsed_out);

      int (*compress_int32)(const scil_context_t* ctx, int32_t * restrict data_out, byte*restrict header, int * header_size_out, int32_t*restrict data_in, const scil_dims_t* dims);
      int (*decompress_int32)(int32_t*restrict data_out, scil_dims_t* dims, int32_t*restrict compressed_buf_in, byte*restrict header_end, int * header_parsed_out);

      int (*compress_int64)(const scil_context_t* ctx, int64_t* restrict data_out, byte*restrict header, int * header_size_out, int64_t*restrict data_in, const scil_dims_t* dims);
      int (*decompress_int64)(int64_t*restrict data_out, scil_dims_t* dims, int64_t*restrict compressed_buf_in, byte*restrict header_end, int * header_parsed_out);
  } PFtype; // preconditioner first stage

    struct{
      // Converter from different datatypes to int64_t i.e. quantize
      int (*compress_float)(const scil_context_t* ctx, int64_t* restrict compressed_buf_in_out, size_t* restrict out_size, float*restrict data_in, const scil_dims_t* dims);
      int (*decompress_float)(float*restrict data_out, scil_dims_t* dims, int64_t*restrict compressed_buf_in, const size_t in_size);

      int (*compress_double)(const scil_context_t* ctx, int64_t* restrict compressed_buf_in_out, size_t* restrict out_size, double*restrict data_in, const scil_dims_t* dims);
      int (*decompress_double)( double*restrict data_out, scil_dims_t* dims, int64_t*restrict compressed_buf_in, const size_t in_size);

      int (*compress_int8)(const scil_context_t* ctx, int64_t* restrict compressed_buf_in_out, size_t* restrict out_size, int8_t*restrict data_in, const scil_dims_t* dims);
      int (*decompress_int8)( int8_t*restrict data_out, scil_dims_t* dims, int64_t*restrict compressed_buf_in, const size_t in_size);

      int (*compress_int16)(const scil_context_t* ctx, int64_t* restrict compressed_buf_in_out, size_t* restrict out_size, int16_t*restrict data_in, const scil_dims_t* dims);
      int (*decompress_int16)( int16_t*restrict data_out, scil_dims_t* dims, int64_t*restrict compressed_buf_in, const size_t in_size);

      int (*compress_int32)(const scil_context_t* ctx, int64_t* restrict compressed_buf_in_out, size_t* restrict out_size, int32_t*restrict data_in, const scil_dims_t* dims);
      int (*decompress_int32)( int32_t*restrict data_out, scil_dims_t* dims, int64_t*restrict compressed_buf_in, const size_t in_size);

      int (*compress_int64)(const scil_context_t* ctx, int64_t* restrict compressed_buf_in_out, size_t* restrict out_size, int64_t*restrict data_in, const scil_dims_t* dims);
      int (*decompress_int64)( int64_t*restrict data_out, scil_dims_t* dims, int64_t*restrict compressed_buf_in, const size_t in_size);
    } Ctype; // converter

    struct{
      // for a preconditioner second stage, we expect that the input buffer points only to the ND data, the output data contains
      // the header of the size as returned and then the preconditioned data.
      int (*compress)(const scil_context_t* ctx, int64_t* restrict data_out, byte*restrict header, int * header_size_out, int64_t*restrict data_in, const scil_dims_t* dims);
      // it is the responsiblity of the decompressor to strip the header that is part of compressed_buf_in
      int (*decompress)(int64_t*restrict data_out, scil_dims_t* dims, int64_t*restrict compressed_buf_in, byte*restrict header_end, int * header_parsed_out);
  } PStype; // preconditioner second stage

    struct{
      int (*compress_float)(const scil_context_t* ctx, byte* restrict compressed_buf_in_out, size_t* restrict out_size, float*restrict data_in, const scil_dims_t* dims);
      int (*decompress_float)(float*restrict data_out, scil_dims_t* dims, byte*restrict compressed_buf_in, const size_t in_size);

      int (*compress_double)(const scil_context_t* ctx, byte* restrict compressed_buf_in_out, size_t* restrict out_size, double*restrict data_in, const scil_dims_t* dims);
      int (*decompress_double)( double*restrict data_out, scil_dims_t* dims, byte*restrict compressed_buf_in, const size_t in_size);

      int (*compress_int8)(const scil_context_t* ctx, byte* restrict compressed_buf_in_out, size_t* restrict out_size, int8_t*restrict data_in, const scil_dims_t* dims);
      int (*decompress_int8)( int8_t*restrict data_out, scil_dims_t* dims, byte*restrict compressed_buf_in, const size_t in_size);

      int (*compress_int16)(const scil_context_t* ctx, byte* restrict compressed_buf_in_out, size_t* restrict out_size, int16_t*restrict data_in, const scil_dims_t* dims);
      int (*decompress_int16)( int16_t*restrict data_out, scil_dims_t* dims, byte*restrict compressed_buf_in, const size_t in_size);

      int (*compress_int32)(const scil_context_t* ctx, byte* restrict compressed_buf_in_out, size_t* restrict out_size, int32_t*restrict data_in, const scil_dims_t* dims);
      int (*decompress_int32)( int32_t*restrict data_out, scil_dims_t* dims, byte*restrict compressed_buf_in, const size_t in_size);

      int (*compress_int64)(const scil_context_t* ctx, byte* restrict compressed_buf_in_out, size_t* restrict out_size, int64_t*restrict data_in, const scil_dims_t* dims);
      int (*decompress_int64)( int64_t*restrict data_out, scil_dims_t* dims, byte*restrict compressed_buf_in, const size_t in_size);
    } DNtype;

    struct{
      int (*compress)(const scil_context_t* ctx, byte* restrict compressed_buf_in_out, size_t* restrict out_size, const byte*restrict data_in, const size_t in_size);
      int (*decompress)(byte*restrict data_out, size_t buff_out_size,  const byte*restrict compressed_buf_in, const size_t in_size, size_t * uncomp_size_out);
    } Btype;

    // TODO: Implement this
    struct{
      int i;
    } ICOtype;
  } c;

  const char * name;
  byte compressor_id;

  enum compressor_type type;
  char is_lossy; // byte compressors are expected to be lossless anyway
} scilU_algorithm_t;

void scil_initialize_compressors();

scilU_algorithm_t* scil_get_compressor(int number);
scilU_algorithm_t* scilU_find_compressor_by_name(const char* name);

/*
 \brief Returns the number of available compression schemes.
 */
int scilU_get_available_compressor_count();

const char* scilU_get_compressor_name(int number);
int scilU_get_compressor_number(const char* name);

#endif // SCIL_ALGOS_H
