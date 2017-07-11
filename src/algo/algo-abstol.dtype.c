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

#define SCIL_ABSTOL_HEADER_SIZE 25
#define SCIL_SMALL 0.00000000000000000000001

static uint64_t round_up_byte(const uint64_t bits){

    uint8_t a = bits % 8;
    if(a == 0)
        return bits / 8;
    return 1 + (bits - a) / 8;
}

static void read_header(const byte* source,
                        size_t* source_size,
                        double* minimum,
                        double* absolute_tolerance,
                        uint8_t* bits_per_value,
                        scil_user_hints_t * hints){
    scilU_unpack8(source, minimum);
    source += 8;
    *source_size -= 8;

    scilU_unpack8(source, absolute_tolerance);
    source += 8;
    *source_size -= 8;

    scilU_unpack1(source, bits_per_value);
    source += 1;
    *source_size -= 1;

    scilU_unpack8(source, & hints->fill_value);
    source += 8;
    *source_size -= 8;
}

static void write_header(byte* dest,
                         double minimum,
                         double absolute_tolerance,
                         uint8_t bits_per_value,
                         const scil_user_hints_t * hints){
    scilU_pack8(dest, minimum);
    dest += 8;

    scilU_pack8(dest, absolute_tolerance);
    dest += 8;

    scilU_pack1(dest, bits_per_value);
    dest += 1;

    scilU_pack8(dest, hints->fill_value);
    dest += 8;
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
    size_t count = scilPr_get_dims_count(dims);

    // Finding minimum and maximum values in data
    <DATATYPE> min, max;
    scilU_find_minimum_maximum_with_excluded_points_<DATATYPE>(source, count, &min, &max, ctx->hints.lossless_data_range_up_to,  ctx->hints.lossless_data_range_from, ctx->hints.fill_value);

    // Locally assigning absolute tolerance
    double abs_tol = ctx->hints.absolute_tolerance * 1.95; // prevent rounding errors

    int next_free_number;
    int reserved = 0;
    // Get needed bits per compressed number in data
    uint64_t bits_per_value = scil_calculate_bits_needed_<DATATYPE>(min, max, abs_tol, reserved, & next_free_number);

    // See if abstol compression makes sense
    if(bits_per_value >= 8 * sizeof(<DATATYPE>)){
        return SCIL_PRECISION_ERR;
    }

    // Get number of needed bytes for the whole compressed buffer
    *dest_size = round_up_byte(bits_per_value * count) + SCIL_ABSTOL_HEADER_SIZE;

    uint64_t* quantized_buffer = (uint64_t*)SAFE_MALLOC(count * sizeof(uint64_t));
    // ==================== Compress ==========================================
    write_header(dest, min, abs_tol, bits_per_value, & ctx->hints);
    dest += SCIL_ABSTOL_HEADER_SIZE;

    // Use quantization to reduce each values bit count
    if(scil_quantize_buffer_minmax_<DATATYPE>(quantized_buffer, source, count, abs_tol, min, max)){
        return SCIL_BUFFER_ERR;
    }

    // Pack data in quantized buffer tightly
    if(scil_swage(dest, quantized_buffer, count, (uint8_t)bits_per_value)){
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
    scil_user_hints_t hints;
    assert(dest != NULL);
    assert(source != NULL);
    assert(dims != NULL);

    double min, abs_tol;
    uint8_t bits_per_value;
    byte* in = source;
    size_t in_size = source_size;
    size_t count = scilPr_get_dims_count(dims);
    uint64_t* unswaged_buffer = (uint64_t*)SAFE_MALLOC(count * sizeof(uint64_t*));

    // ============ Decompress ================================================
    // Parse Header
    read_header(in, &in_size, &min, &abs_tol, &bits_per_value, &hints);
    in += SCIL_ABSTOL_HEADER_SIZE;

    // Unpacking buffer
    if(scil_unswage(unswaged_buffer, in, count, bits_per_value)){
        return SCIL_BUFFER_ERR;
    }

    // Unquantizing buffer
    if(scil_unquantize_buffer_<DATATYPE>(dest, unswaged_buffer, count, abs_tol, min)){
        return SCIL_BUFFER_ERR;
    }
    // ========================================================================

    free(unswaged_buffer);

    return SCIL_NO_ERR;
}
// End repeat



scilI_algorithm_t algo_abstol = {
    .c.DNtype = {
        CREATE_INITIALIZER(scil_abstol)
    },
    "abstol",
    1,
    SCIL_COMPRESSOR_TYPE_DATATYPES,
    1
};
