// This file is part of SCIL.
//
// SCIL is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SCIL is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FpITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with SCIL.  If not, see <http://www.gnu.org/licenses/>.

#include <algo/algo-abstol.h>

#include <scil-quantizer.h>
#include <scil-swager.h>
#include <scil-util.h>

#include <assert.h>
#include <math.h>
#include <string.h>

static uint64_t round_up_byte(const uint64_t bits){

    uint8_t a = bits % 8;
    if(a == 0)
        return bits / 8;
    return 1 + (bits - a) / 8;
}

static int read_header(const byte* source,
                        size_t* source_size,
                        double* minimum,
                        double* absolute_tolerance,
                        uint8_t* bits_per_value,
                        double * fill_value,
                        uint64_t * next_free_number){
  const byte * start = source;
  scilU_unpack8(source, minimum);
  source += 8;

  scilU_unpack8(source, absolute_tolerance);
  source += 8;

  scilU_unpack1(source, bits_per_value);
  source += 1;

  scilU_unpack8(source, fill_value);
  source += 8;

  if(*fill_value != DBL_MAX){
    *next_free_number = *(uint64_t*)source;
    source += 8;
  }

  int size = (int) (source - start);
  *source_size -= size;
  return size;
}

static int write_header(byte* dest, double minimum, double absolute_tolerance, uint8_t bits_per_value, double fill_value, uint64_t next_free_number){
  byte * start = dest;
  scilU_pack8(dest, minimum);
  dest += 8;

  scilU_pack8(dest, absolute_tolerance);
  dest += 8;

  scilU_pack1(dest, bits_per_value);
  dest += 1;

  scilU_pack8(dest, fill_value);
  dest += 8;

  if (fill_value != DBL_MAX){
    *(uint64_t*)dest = next_free_number;
    dest += 8;
  }
  return (int) (dest - start);
}

//Repeat for each data type
//Supported datatypes: double float int8_t int16_t int32_t int64_t

int scil_abstol_compress_<DATATYPE>(const scil_context_t* ctx,
                                    byte* restrict dest,
                                    size_t* restrict dest_size,
                                    <DATATYPE>* restrict source,
                                    const scil_dims_t* dims){
    assert(dest != NULL);
    assert(dest_size != NULL);
    assert(source != NULL);
    assert(dims != NULL);

    // Element count in buffer to compress
    size_t count = scil_dims_get_count(dims);

    // Finding minimum and maximum values in data
    <DATATYPE> min, max;
    scilU_find_minimum_maximum_with_excluded_points_<DATATYPE>(source, count, &min, &max, ctx->hints.lossless_data_range_up_to,  ctx->hints.lossless_data_range_from, ctx->hints.fill_value);

    // Locally assigning absolute tolerance
    double abs_tol = ctx->hints.absolute_tolerance; // prevent rounding errors

    uint64_t next_free_number;
    int reserved = 0;
    // Get needed bits per compressed number in data
    uint8_t bits_per_value = scil_calculate_bits_needed_<DATATYPE>(min, max, abs_tol, reserved, & next_free_number);

    // See if abstol compression makes sense
    if(bits_per_value >= 8 * sizeof(<DATATYPE>)){
        return SCIL_PRECISION_ERR;
    }

    if(bits_per_value == 0){
      // special case, constant, may be set to the MEAN value across all points
      min = (max + min) / 2.0;
    }

    // Get number of needed bytes for the whole compressed buffer

    // ==================== Compress ==========================================
    int header_size = write_header(dest, min, abs_tol, bits_per_value, ctx->hints.fill_value, next_free_number);
    dest += header_size;
    if(bits_per_value == 0){
      // constant, stop here
      *dest_size = header_size;
      return SCIL_NO_ERR;
    }
    *dest_size = round_up_byte((uint64_t)bits_per_value * count) + header_size;

    uint64_t* quantized_buffer = (uint64_t*)scilU_safe_malloc(count * sizeof(uint64_t));

    if (ctx->hints.fill_value == DBL_MAX){
      // Use quantization to reduce each values bit count
      if(scil_quantize_buffer_minmax_<DATATYPE>(quantized_buffer, source, count, abs_tol, min, max)){
          return SCIL_BUFFER_ERR;
      }
    }else{ // use the fill value
      if(scil_quantize_buffer_minmax_fill_<DATATYPE>(quantized_buffer, source, count, abs_tol, min, max, ctx->hints.fill_value, next_free_number)){
          return SCIL_BUFFER_ERR;
      }
    }

    // Pack data in quantized buffer tightly
    if(scil_swage(dest, quantized_buffer, count, bits_per_value)){
        return SCIL_BUFFER_ERR;
    }
    // ========================================================================

    free(quantized_buffer);

    return SCIL_NO_ERR;
}

int scil_abstol_decompress_<DATATYPE>(<DATATYPE>* restrict dest,
                                      scil_dims_t* dims,
                                      byte* restrict source,
                                      size_t source_size){
    assert(dest != NULL);
    assert(source != NULL);
    assert(dims != NULL);

    double min, abs_tol;
    double fill_value = DBL_MAX;
    uint8_t bits_per_value;
    byte* in = source;
    size_t in_size = source_size;
    size_t count = scil_dims_get_count(dims);
    uint64_t next_free_number;

    // ============ Decompress ================================================
    // Parse Header
    in += read_header(in, &in_size, &min, &abs_tol, &bits_per_value, &fill_value, & next_free_number);
    if(bits_per_value == 0){
      for(size_t i = 0; i < count; ++i){
        dest[i] = (<DATATYPE>) min;
      }
      return SCIL_NO_ERR;
    }

    uint64_t* unswaged_buffer = (uint64_t*)scilU_safe_malloc(count * sizeof(uint64_t*));
    // Unpacking buffer
    if(scil_unswage(unswaged_buffer, in, count, bits_per_value)){
        return SCIL_BUFFER_ERR;
    }

    if (fill_value == DBL_MAX){
      // Unquantizing buffer
      if(scil_unquantize_buffer_<DATATYPE>(dest, unswaged_buffer, count, abs_tol, min)){
          return SCIL_BUFFER_ERR;
      }
    }else{
      if(scil_unquantize_buffer_fill_<DATATYPE>(dest, unswaged_buffer, count, abs_tol, min, fill_value, next_free_number)){
          return SCIL_BUFFER_ERR;
      }
    }
    // ========================================================================

    free(unswaged_buffer);

    return SCIL_NO_ERR;
}
// End repeat



scilU_algorithm_t algo_abstol = {
    .c.DNtype = {
        CREATE_INITIALIZER(scil_abstol)
    },
    "abstol",
    1,
    SCIL_COMPRESSOR_TYPE_DATATYPES,
    1
};
