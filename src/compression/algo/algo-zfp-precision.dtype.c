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

#include <string.h>
#include <math.h>
#include <assert.h>

#include <algo/algo-zfp-precision.h>

#include <zfp.h>

#include <scil-util.h>


//Supported datatypes: float double
// Repeat for each data type

static void find_minimums_and_maximums_<DATATYPE>(const <DATATYPE>* buffer,
                                                  const size_t size,
                                                  uint8_t* minimum_sign,
                                                  uint8_t* maximum_sign,
                                                  int16_t* minimum_exponent,
                                                  int16_t* maximum_exponent){
    *minimum_sign = 1;
    *maximum_sign = 0;

    *minimum_exponent = 0x7fff;
    *maximum_exponent = -*minimum_exponent;

    for(size_t i = 0; i < size; ++i){

        datatype_cast_<DATATYPE> cur;
        cur.f = buffer[i];

        if(*minimum_sign != 0 && cur.p.sign < *minimum_sign) { *minimum_sign = cur.p.sign; }
        if(*maximum_sign != 1 && cur.p.sign > *maximum_sign) { *maximum_sign = cur.p.sign; }

        if(cur.p.exponent < *minimum_exponent) { *minimum_exponent = cur.p.exponent; }
        if(cur.p.exponent > *maximum_exponent) { *maximum_exponent = cur.p.exponent; }
    }
}

#pragma GCC diagnostic ignored "-Wunused-parameter"
int scil_zfp_precision_compress_<DATATYPE>(const scil_context_t* ctx,
                        byte * restrict dest,
                        size_t* restrict dest_size,
                        <DATATYPE>*restrict source,
                        const scil_dims_t* dims)
{
    int ret = 0;

    // Compress
    zfp_field* field = NULL;
    size_t count = scil_dims_get_count(dims);

    switch(dims->dims){
        case 1: field = zfp_field_1d(source, zfp_type_<DATATYPE>, dims->length[0]); break;
        case 2: field = zfp_field_2d(source, zfp_type_<DATATYPE>, dims->length[0], dims->length[1]); break;
        case 3: field = zfp_field_3d(source, zfp_type_<DATATYPE>, dims->length[0], dims->length[1], dims->length[2]); break;
        default: field = zfp_field_1d(source, zfp_type_<DATATYPE>, count);
    }

    // determine number of bits for the exponent
    uint8_t minimum_sign, maximum_sign;
    int16_t maximum_exponent, minimum_exponent;
    find_minimums_and_maximums_<DATATYPE>(source,
                                          count,
                                          &minimum_sign,
                                          &maximum_sign,
                                          & minimum_exponent,
                                          &maximum_exponent);
    uint8_t exponent_bit_count;
    exponent_bit_count = (uint8_t) ceil(log2(maximum_exponent - minimum_exponent + 1));
    if(minimum_sign != maximum_sign){
      exponent_bit_count += 1;
    }

    zfp_stream* zfp = zfp_stream_open(NULL);

    uint precision = ctx->hints.significant_bits + exponent_bit_count; // EXPONENT_LENGTH_<DATATYPE_UPPER>


    *dest_size = 0;
    *((uint*)dest) = precision;
    dest += sizeof(uint);
    *dest_size += sizeof(uint);

    /*  zfp_stream_set_rate(zfp, rate, type, 3, 0); */
    uint actual_precision = zfp_stream_set_precision(zfp, precision, zfp_type_<DATATYPE>);
    //assert(actual_precision == precision);

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

    return ret;
}

#pragma GCC diagnostic ignored "-Wunused-parameter"
int scil_zfp_precision_decompress_<DATATYPE>(   <DATATYPE>*restrict data_out,
                                                scil_dims_t* dims,
                                                byte*restrict compressed_buf_in,
                                                const size_t in_size)
{
    int ret = 0;

    uint precision = *((uint*)compressed_buf_in);
    compressed_buf_in += sizeof(uint);

    // Decompress
    zfp_field* field = NULL;

    switch(dims->dims){
        case 1: field = zfp_field_1d(data_out, zfp_type_<DATATYPE>, dims->length[0]); break;
        case 2: field = zfp_field_2d(data_out, zfp_type_<DATATYPE>, dims->length[0], dims->length[1]); break;
        case 3: field = zfp_field_3d(data_out, zfp_type_<DATATYPE>, dims->length[0], dims->length[1], dims->length[2]); break;
        default: field = zfp_field_1d(data_out, zfp_type_<DATATYPE>, scil_dims_get_count(dims));
    }

    zfp_stream* zfp = zfp_stream_open(NULL);

    /*  zfp_stream_set_rate(zfp, rate, type, 3, 0); */
    zfp_stream_set_precision(zfp, precision, zfp_type_<DATATYPE>);
    /* zfp_stream_set_accuracy(zfp, tolerance, zfp_type_<DATATYPE>); */

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

    return ret;
}

// End repeat

scilU_algorithm_t algo_zfp_precision = {
    .c.DNtype = {
        CREATE_INITIALIZER(scil_zfp_precision)
    },
    "zfp-precision",
    6,
    SCIL_COMPRESSOR_TYPE_DATATYPES,
    1
};
