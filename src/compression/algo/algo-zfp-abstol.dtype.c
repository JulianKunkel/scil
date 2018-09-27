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

#include <algo/algo-zfp-abstol.h>

#include <string.h>

#include <scil-util.h>

#include <zfp.h>

static int read_header(const byte* source,
                        size_t source_size,
                        double * absolute_tolerance,
                        double * fill_value,
                        double * next_free_number){
  const byte * start = source;

  scilU_unpack8(source, absolute_tolerance);
  source += 8;

  scilU_unpack8(source, fill_value);
  source += 8;

  if(*fill_value != DBL_MAX){
    scilU_unpack8(source, next_free_number);
    source += 8;
  }

  int size = (int) (source - start);
  return size;
}

static int write_header(byte* dest, double absolute_tolerance, double fill_value, double next_free_number){
  byte * start = dest;

  scilU_pack8(dest, absolute_tolerance);
  dest += 8;

  scilU_pack8(dest, fill_value);
  dest += 8;

  if (fill_value != DBL_MAX){
    scilU_pack8(dest, next_free_number);
    dest += 8;
  }
  return (int) (dest - start);
}

//Supported datatypes: float double
// Repeat for each data type
#pragma GCC diagnostic ignored "-Wfloat-equal"
#pragma GCC diagnostic ignored "-Wunused-parameter"

int scil_zfp_abstol_compress_<DATATYPE>(const scil_context_t* ctx,
                        byte * restrict dest,
                        size_t* restrict dest_size,
                        <DATATYPE>*restrict source,
                        const scil_dims_t* dims)
{
    int ret = 0;
    *dest_size = 0;

    // Element count in buffer to compress
    size_t count = scil_dims_get_count(dims);

    double next_free_number = 0;

    <DATATYPE>* in = source;

    double abs_tol = (ctx->hints.absolute_tolerance == SCIL_ACCURACY_DBL_FINEST) ? 0 : ctx->hints.absolute_tolerance;

    if (ctx->hints.fill_value != DBL_MAX){
      in = (<DATATYPE>*)scilU_safe_malloc(count * sizeof(<DATATYPE>));
      memcpy(in, source, count * sizeof(<DATATYPE>));

      // Finding minimum and maximum values in data
      <DATATYPE> min, max;
      scilU_find_minimum_maximum_with_excluded_points_<DATATYPE>(in, count, &min, &max, ctx->hints.lossless_data_range_up_to,  ctx->hints.lossless_data_range_from, ctx->hints.fill_value);

      next_free_number = max + 2 * abs_tol;

      const <DATATYPE> fill_value_d = (<DATATYPE>) ctx->hints.fill_value;
      const <DATATYPE> next_free_d = (<DATATYPE>)next_free_number;
      for (int i = 0; i < count; i++){
        if (in[i] == fill_value_d){
          in[i] = next_free_d;
        }
      }
    }

    int header_size = write_header(dest, abs_tol, ctx->hints.fill_value, next_free_number);
    dest += header_size;
    *dest_size += header_size;

    // Compress
    zfp_field* field = NULL;

    switch(dims->dims){
        case 1: field = zfp_field_1d(in, zfp_type_<DATATYPE>, dims->length[0]); break;
        case 2: field = zfp_field_2d(in, zfp_type_<DATATYPE>, dims->length[0], dims->length[1]); break;
        case 3: field = zfp_field_3d(in, zfp_type_<DATATYPE>, dims->length[0], dims->length[1], dims->length[2]); break;
        default: field = zfp_field_1d(in, zfp_type_<DATATYPE>, scil_dims_get_count(dims));
    }

    zfp_stream* zfp = zfp_stream_open(NULL);

    /*  zfp_stream_set_rate(zfp, rate, type, 3, 0); */
    /*  zfp_stream_set_precision(zfp, precision, type); */
    zfp_stream_set_accuracy(zfp, ctx->hints.absolute_tolerance, zfp_type_<DATATYPE>);

    size_t bufsize = zfp_stream_maximum_size(zfp, field);
    bitstream* stream = stream_open(dest, bufsize);
    zfp_stream_set_bit_stream(zfp, stream);
    zfp_stream_rewind(zfp);

    *dest_size += zfp_compress(zfp, field);
    if(*dest_size == 0){
        fprintf(stderr, "ZPF compression failed\n");
        ret = 1;
    }

    zfp_field_free(field);
    zfp_stream_close(zfp);
    stream_close(stream);

    if (ctx->hints.fill_value != DBL_MAX){
      free(in);
    }

    return ret;
}

#pragma GCC diagnostic ignored "-Wunused-parameter"
int scil_zfp_abstol_decompress_<DATATYPE>( <DATATYPE>*restrict data_out,
                            scil_dims_t* dims,
                            byte*restrict compressed_buf_in,
                            const size_t in_size)
{
    int ret = 0;
    double tolerance;
    double fill_value = DBL_MAX;
    double next_free_number;
    int size = in_size;
    size_t count = scil_dims_get_count(dims);

    size -= read_header(compressed_buf_in, in_size, &tolerance, &fill_value, & next_free_number);
    compressed_buf_in += in_size - size;

    // Decompress
    zfp_field* field = NULL;

    switch(dims->dims){
        case 1: field = zfp_field_1d(data_out, zfp_type_<DATATYPE>, dims->length[0]); break;
        case 2: field = zfp_field_2d(data_out, zfp_type_<DATATYPE>, dims->length[0], dims->length[1]); break;
        case 3: field = zfp_field_3d(data_out, zfp_type_<DATATYPE>, dims->length[0], dims->length[1], dims->length[2]); break;
        default: field = zfp_field_1d(data_out, zfp_type_<DATATYPE>, count);
    }

    zfp_stream* zfp = zfp_stream_open(NULL);

    /*  zfp_stream_set_rate(zfp, rate, type, 3, 0); */
    /*  zfp_stream_set_precision(zfp, precision, type); */
    zfp_stream_set_accuracy(zfp, tolerance, zfp_type_<DATATYPE>);

    size_t bufsize = zfp_stream_maximum_size(zfp, field);
    bitstream* stream = stream_open(compressed_buf_in, bufsize);
    zfp_stream_set_bit_stream(zfp, stream);
    zfp_stream_rewind(zfp);

    if(!zfp_decompress(zfp, field)){
        fprintf(stderr, "ZPF compression failed\n");
        ret = 1;
    }

    zfp_field_free(field);
    zfp_stream_close(zfp);
    stream_close(stream);

    if (fill_value != DBL_MAX){
      const <DATATYPE> fill_value_d = (<DATATYPE>)fill_value;
      for (int i = 0; i < count; i++){
        if (fabs(next_free_number - data_out[i]) <= tolerance){
          data_out[i] = fill_value_d;
        }
      }
    }

    return ret;
}

// End repeat

scilU_algorithm_t algo_zfp_abstol = {
    .c.DNtype = {
        CREATE_INITIALIZER(scil_zfp_abstol)
    },
    "zfp-abstol",
    5,
    SCIL_COMPRESSOR_TYPE_DATATYPES,
    1
};
