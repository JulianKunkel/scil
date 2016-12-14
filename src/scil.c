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
#include <scil-algo-chooser.h>
#include <scil-error.h>
#include <scil-hardware-limits.h>
#include <scil-internal.h>
#include <scil-context.h>

#include <scil-compressors.h>

#include <ctype.h>
#include <float.h>
#include <math.h>
#include <string.h>

// this file is automatically created
#include "scil-dtypes.h"
#include "scil-dtypes-int.h"

#define CHECK_COMPRESSOR_ID(compressor_id)               \
    if (compressor_id >= scilU_get_available_compressor_count()) { \
        return SCIL_BUFFER_ERR;                          \
    }

void scil_compression_sprint_last_algorithm_chain(scil_context_t* ctx, char* out, int buff_length)
{
    int ret                      = 0;
    scilI_chain_t* lc = &ctx->chain;
    for (int i = 0; i < PRECONDITIONER_LIMIT; i++) {
        if (lc->pre_cond_first[i] == NULL) break;
        ret = snprintf(out, buff_length, "%s,", lc->pre_cond_first[i]->name);
        buff_length -= ret;
        out += ret;
    }
	if (lc->converter != NULL) {
		ret = snprintf(out, buff_length, "%s,", lc->converter->name);
		buff_length -= ret;
		out += ret;
	}
	for (int i = 0; i < PRECONDITIONER_LIMIT; i++) {
        if (lc->pre_cond_second[i] == NULL) break;
        ret = snprintf(out, buff_length, "%s,", lc->pre_cond_second[i]->name);
        buff_length -= ret;
        out += ret;
    }
	if (lc->data_compressor != NULL) {
		ret = snprintf(out, buff_length, "%s,", lc->data_compressor->name);
		buff_length -= ret;
		out += ret;
	}
    if (lc->byte_compressor != NULL) {
        ret = snprintf(out, buff_length, "%s,", lc->byte_compressor->name);
        buff_length -= ret;
        out += ret;
    }
    // remove the last character
    out[-1] = 0;
}

static inline void* pick_buffer(int is_src,
                                int total,
                                int remain,
                                void* restrict src,
                                void* restrict dest,
                                void* restrict buff1,
                                void* restrict buff2)
{
    if (remain == total && is_src) {
        return src;
    }
    if (remain == 1 && !is_src) {
        return dest;
    }
    if (remain % 2 == is_src) {
        return buff1;
    } else {
        return buff2;
    }
}

/*
A compression chain compresses data in multiple phases, i.e., applying algo 1,
then algo 2 ...
The processing sequence may consist of the following steps:
1) a sequence of datatype preconditioners
2) a data compressor
3) a single byte compressor.
For cache efficiency reasons, a compound compression scheme should be used
instead of multiple data copy stages.

Internally, the compressed buffer is formated as follows:
- byte CHAIN_LENGTH // the number of compressors to apply.

Then for the last compressor that has been applied the format looks like:
- byte compressor_id // The compressor number as registered in SCIL.
- byte * COMPRESSOR_SPECIFIC_HEADER
- byte * COMPRESSED DATA

If the chain consists of multiple compressors (n-many) the final format looks
like:

byte CHAIN_LENGTH
byte compressor_id_ALGO(n)
ALGO(n) specific header
ALGO(n-1) data compressed using ALGO(n)

If ALGO(n-1) is a datatype specific algorithm, then it usually cannot handle
arbitrary bytes.
Therefore, the compressor ID and headers of nested datatypes must be split from
the data.

A datatype compressor terminates the chain of preconditioners.
 */
int scil_compress(byte* restrict dest,
                  size_t in_dest_size,
                  void* restrict source,
                  scil_dims_t* dims,
                  size_t* restrict out_size_p,
                  scil_context_t* ctx) {

	assert(ctx != NULL);
	assert(dest != NULL);
	assert(out_size_p != NULL);
	assert(source != NULL);

	int ret = SCIL_NO_ERR;

	// Get byte size of input data
    size_t input_size           = scilPr_get_dims_size(dims, ctx->datatype);
    const size_t datatypes_size = input_size;

	// Skip the compression if input size is 0 and set destination buffer to a single 0 and size 1
    if (datatypes_size == 0) {
        out_size_p[0] = 1;
        dest[0]       = (byte)0;

        return SCIL_NO_ERR;
    }

	// Why? Factor 4 seems arbitrary
    if (in_dest_size < 4 * datatypes_size) {
        return SCIL_MEMORY_ERR;
    }

	// Set local references of hints and compression chain
    const scil_user_hints_t* hints = &ctx->hints;
    scilI_chain_t* chain  = &ctx->chain;

	// Check whether automatic compressor decision can be skipped because of a user forced chain
    if (hints->force_compression_methods == NULL) {
        scilC_algo_chooser_execute(source, dims, ctx);
    }

    size_t out_size = 0;

    // Add the length of the algo chain to the output
    int remaining_compressors   = chain->total_size;
    const int total_compressors = remaining_compressors;
    dest[0]                     = total_compressors;
    dest++;

    // Process the compression pipeline
    // we use 1.5 the memory buffer as intermediate location // no we don't, do we?
    const size_t buffer_tmp_offset = 2 * datatypes_size + 10;
    byte* restrict buff_tmp        = &dest[buffer_tmp_offset];

    // process the compression chain
    // apply the first pre-conditioners
    if (chain->precond_first_count > 0) {
        out_size += datatypes_size;
        // add the header at the end of the preconditioners
        byte* header = datatypes_size + (byte*)pick_buffer(0, total_compressors, 1 + total_compressors - chain->precond_first_count, source, dest, buff_tmp, dest);

        for (int i = 0; i < chain->precond_first_count; i++) {
            int header_size_out;
            scilI_algorithm_t* algo = chain->pre_cond_first[i];
            void* src = pick_buffer(1, total_compressors, remaining_compressors, source, dest, buff_tmp, dest);
            void* dst = pick_buffer(0, total_compressors, remaining_compressors, source, dest, buff_tmp, dest);

            switch (ctx->datatype) {
                case (SCIL_TYPE_FLOAT):
                    ret = algo->c.PFtype.compress_float(ctx, (float*)dst, header, &header_size_out, src, dims);
                    break;
                case (SCIL_TYPE_DOUBLE):
                    ret = algo->c.PFtype.compress_double(ctx, (double*)dst, header, &header_size_out, src, dims);
                    break;
				case (SCIL_TYPE_INT8) :
					ret = algo->c.PFtype.compress_int8(ctx, (int8_t*)dst, header, &header_size_out, src, dims);
					break;
				case(SCIL_TYPE_INT16) :
					ret = algo->c.PFtype.compress_int16(ctx, (int16_t*)dst, header, &header_size_out, src, dims);
					break;
				case(SCIL_TYPE_INT32) :
					ret = algo->c.PFtype.compress_int32(ctx, (int32_t*)dst, header, &header_size_out, src, dims);
					break;
				case(SCIL_TYPE_INT64) :
					ret = algo->c.PFtype.compress_int64(ctx, (int64_t*)dst, header, &header_size_out, src, dims);
					break;
				case(SCIL_TYPE_STRING) :
					assert(0);
					break;
            }

            if (ret != 0) return ret;
            remaining_compressors--;
            out_size += header_size_out;
            header   += header_size_out;

            *header = algo->compressor_id;
            debugI(
                "C compressor ID %d at pos %llu\n", *header, (long long unsigned)header)
                header++;
            out_size++;

            // scilU_print_buffer(dst, out_size);
        }
        input_size = out_size;
    }

	// Apply the converter
	if (chain->converter) {
		// we need to preserve the header of the pre-conditioners.
        void* src = pick_buffer(1, total_compressors, remaining_compressors, source, dest, buff_tmp, dest);
        void* dst = pick_buffer(0, total_compressors, remaining_compressors, source, dest, buff_tmp, dest);

        // set the output size to the expected buffer size
        out_size = (size_t)(datatypes_size * 2);

        scilI_algorithm_t* algo = chain->converter;
        switch (ctx->datatype) {
            case (SCIL_TYPE_FLOAT):
                ret = algo->c.Ctype.compress_float(ctx, (int64_t*)dst, &out_size, src, dims);
                break;
            case (SCIL_TYPE_DOUBLE):
                ret = algo->c.Ctype.compress_double(ctx, (int64_t*)dst, &out_size, src, dims);
                break;
			case (SCIL_TYPE_INT8) :
				ret = algo->c.Ctype.compress_int8(ctx, (int64_t*)dst, &out_size, src, dims);
				break;
			case(SCIL_TYPE_INT16) :
				ret = algo->c.Ctype.compress_int16(ctx, (int64_t*)dst, &out_size, src, dims);
				break;
			case(SCIL_TYPE_INT32) :
				ret = algo->c.Ctype.compress_int32(ctx, (int64_t*)dst, &out_size, src, dims);
				break;
			case(SCIL_TYPE_INT64) :
				ret = algo->c.Ctype.compress_int64(ctx, (int64_t*)dst, &out_size, src, dims);
				break;
			case(SCIL_TYPE_STRING) :
				assert(0);
				break;
        }
        if (ret != 0) return ret;
        // check if we have to preserve another header from the preconditioners
        if (datatypes_size != input_size) {
            // we have to copy some header.
            debugI("Preserving %lld %lld\n", (long long)datatypes_size, (long long)input_size);
            const int preserve = input_size - datatypes_size;
            memcpy((char*)dst + out_size, (char*)src + datatypes_size, preserve); // TODO: potential error
            out_size += preserve;
            scilU_print_buffer(dst, out_size);
        }

        remaining_compressors--;
        ((char*)dst)[out_size] = algo->compressor_id;
        debugI("C compressor ID %d at pos %llu\n",
               algo->compressor_id,
               (long long unsigned)&((char*)dst)[out_size]);

        out_size++;
        input_size = out_size;
	}

	// apply the second pre-conditioners
    if (chain->precond_second_count > 0) {
        out_size += datatypes_size;
        // add the header at the end of the preconditioners
        byte* header = datatypes_size + (byte*)pick_buffer(0, total_compressors, 1 + total_compressors - chain->precond_second_count, source, dest, buff_tmp, dest);

        for (int i = 0; i < chain->precond_second_count; i++) {
            int header_size_out;
            scilI_algorithm_t* algo = chain->pre_cond_second[i];
            void* src = pick_buffer(1, total_compressors, remaining_compressors, source, dest, buff_tmp, dest);
            void* dst = pick_buffer(0, total_compressors, remaining_compressors, source, dest, buff_tmp, dest);

			ret = algo->c.PStype.compress(ctx, (int64_t*)dst, header, &header_size_out, src, dims);

            if (ret != 0) return ret;
            remaining_compressors--;
            out_size += header_size_out;
            header   += header_size_out;

            *header = algo->compressor_id;
            debugI(
                "C compressor ID %d at pos %llu\n", *header, (long long unsigned)header)
                header++;
            out_size++;

            // scilU_print_buffer(dst, out_size);
        }
        input_size = out_size;
    }

	// Apply the data compressor
    if (chain->data_compressor) {
        // we need to preserve the header of the pre-conditioners.
        void* src = pick_buffer(1, total_compressors, remaining_compressors, source, dest, buff_tmp, dest);
        void* dst = pick_buffer(0, total_compressors, remaining_compressors, source, dest, buff_tmp, dest);

        // set the output size to the expected buffer size
        out_size = (size_t)(datatypes_size * 2);

        scilI_algorithm_t* algo = chain->data_compressor;
        switch (ctx->datatype) {
            case (SCIL_TYPE_FLOAT):
                ret = algo->c.DNtype.compress_float(ctx, dst, &out_size, src, dims);
                break;
            case (SCIL_TYPE_DOUBLE):
                ret = algo->c.DNtype.compress_double(ctx, dst, &out_size, src, dims);
                break;
			case (SCIL_TYPE_INT8) :
				ret = algo->c.DNtype.compress_int8(ctx, dst, &out_size, src, dims);
				break;
			case(SCIL_TYPE_INT16) :
				ret = algo->c.DNtype.compress_int16(ctx, dst, &out_size, src, dims);
				break;
			case(SCIL_TYPE_INT32) :
				ret = algo->c.DNtype.compress_int32(ctx, dst, &out_size, src, dims);
				break;
			case(SCIL_TYPE_INT64) :
				ret = algo->c.DNtype.compress_int64(ctx, dst, &out_size, src, dims);
				break;
			case(SCIL_TYPE_STRING) :
				assert(0);
				break;
        }
        if (ret != 0) return ret;
        // check if we have to preserve another header from the preconditioners
        if (datatypes_size != input_size) {
            // we have to copy some header.
            debugI("Preserving %lld %lld\n", (long long)datatypes_size, (long long)input_size);
            const int preserve = input_size - datatypes_size;
            memcpy((char*)dst + out_size, (char*)src + datatypes_size, preserve);
            out_size += preserve;
            scilU_print_buffer(dst, out_size);
        }

        remaining_compressors--;
        ((char*)dst)[out_size] = algo->compressor_id;
        debugI("C compressor ID %d at pos %llu\n",
               algo->compressor_id,
               (long long unsigned)&((char*)dst)[out_size]);

        out_size++;
        input_size = out_size;
    }

	// Apply byte compressor
    if (chain->byte_compressor) {
        void* src = pick_buffer(1, total_compressors, remaining_compressors, source, dest, buff_tmp, dest);

        // scilU_print_buffer(src, input_size);

        ret = chain->byte_compressor->c.Btype.compress(ctx, dest, &out_size, (byte*)src, input_size);
        if (ret != 0) return ret;
        dest[out_size] = chain->byte_compressor->compressor_id;
        debugI("C compressor ID %d at pos %llu\n",
               chain->byte_compressor->compressor_id,
               (long long unsigned)&dest[out_size]);

        out_size++;
        // scilU_print_buffer(dest, out_size);
    }

    *out_size_p = out_size + 1; // for the length of the processing chain
    return SCIL_NO_ERR;
}

int scil_decompress(enum SCIL_Datatype datatype,
                    void* restrict dest,
                    scil_dims_t* dims,
                    byte* restrict source,
                    const size_t source_size,
                    byte* restrict buff_tmp1) {

    if (dims->dims == 0) {
        return SCIL_NO_ERR;
    }

    assert(dest != NULL);
    assert(source != NULL);
    assert(buff_tmp1 != NULL);

    // Read compressor ID (algorithm id) from header
    const int total_compressors = (uint8_t)source[0];
    int remaining_compressors   = total_compressors;

    byte* restrict src_adj = source + 1;
    size_t src_size        = source_size - 1;
    int ret;

    const size_t output_size = scilPr_get_dims_size(dims, datatype);
    byte* restrict buff_tmp2 = &buff_tmp1[(int)(output_size * 2 + 10)];

    // for(int i=0; i < chain_size; i++){
    src_size--;
    uint8_t compressor_id = src_adj[src_size];
    debugI("D compressor ID %d at pos %llu\n", compressor_id, (long long unsigned)&src_adj[src_size]);

    CHECK_COMPRESSOR_ID(compressor_id)
    // printf("SCHUH %d %lld\n", compressor_id, source_size);

    scilI_algorithm_t* algo = algo_array[compressor_id];
    byte* header                     = &src_adj[src_size - 1];

    if (algo->type == SCIL_COMPRESSOR_TYPE_INDIVIDUAL_BYTES) {
        void* src = pick_buffer(1, total_compressors, remaining_compressors, src_adj, dest, buff_tmp1, buff_tmp2);
        void* dst = pick_buffer(0, total_compressors, remaining_compressors, src_adj, dest, buff_tmp1, buff_tmp2);

        ret = algo->c.Btype.decompress(dst, output_size * 2 + 10, (byte*)src, src_size, &src_size);
        if (ret != 0) return ret;
        remaining_compressors--;

        // the header is on the right hand side of the buffer
        header = &((byte*)dst)[src_size];

        if (remaining_compressors > 0) {
            // scilU_print_buffer(dst, src_size);

            header--;
            compressor_id = header[0];
            header--;

            debugI("D compressor ID %d at pos %llu\n",
                   compressor_id,
                   (long long unsigned)header);
            CHECK_COMPRESSOR_ID(compressor_id)
            algo = algo_array[compressor_id];
        }
    }

    if (algo->type == SCIL_COMPRESSOR_TYPE_DATATYPES) {
        void* src = pick_buffer(1, total_compressors, remaining_compressors, src_adj, dest, buff_tmp1, buff_tmp2);
        void* dst = pick_buffer(0, total_compressors, remaining_compressors, src_adj, dest, buff_tmp1, buff_tmp2);

        switch (datatype) {
            case (SCIL_TYPE_FLOAT):
                ret = algo->c.DNtype.decompress_float(dst, dims, src, src_size);
                break;
            case (SCIL_TYPE_DOUBLE):
                ret = algo->c.DNtype.decompress_double(dst, dims, src, src_size);
                break;
			case (SCIL_TYPE_INT8) :
				ret = algo->c.DNtype.decompress_int8(dst, dims, src, src_size);
				break;
			case(SCIL_TYPE_INT16) :
				ret = algo->c.DNtype.decompress_int16(dst, dims, src, src_size);
				break;
			case(SCIL_TYPE_INT32) :
				ret = algo->c.DNtype.decompress_int32(dst, dims, src, src_size);
				break;
			case(SCIL_TYPE_INT64) :
				ret = algo->c.DNtype.decompress_int64(dst, dims, src, src_size);
				break;
			case(SCIL_TYPE_STRING) :
				assert(0);
				break;
        }

        if (ret != 0) return ret;
        remaining_compressors--;
        if (remaining_compressors > 0) {
            // scilU_print_buffer(dst, src_size);
            compressor_id = *((char*)header);
            header--;
            CHECK_COMPRESSOR_ID(compressor_id)
            algo = algo_array[compressor_id];
        }
    }

	while (algo->type == SCIL_COMPRESSOR_TYPE_DATATYPES_PRECONDITIONER_SECOND)
	{
        void* src = pick_buffer(1, total_compressors, remaining_compressors, src_adj, dest, buff_tmp1, buff_tmp2);
        void* dst = pick_buffer(0, total_compressors, remaining_compressors, src_adj, dest, buff_tmp1, buff_tmp2);
        int header_parsed;

        ret = algo->c.PStype.decompress(dst, dims, src, header, &header_parsed);

        header -= header_parsed;

        if (ret != 0) return ret;
        remaining_compressors--;

        // scilU_print_buffer(dst, src_size);
        compressor_id = *((char*)header);
        debugI("D compressor ID %d at pos %llu\n", compressor_id, (long long unsigned)header);
        header--;
        CHECK_COMPRESSOR_ID(compressor_id)
        algo = algo_array[compressor_id];
    }

	if (algo->type == SCIL_COMPRESSOR_TYPE_DATATYPES_CONVERTER) {
        void* src = pick_buffer(1, total_compressors, remaining_compressors, src_adj, dest, buff_tmp1, buff_tmp2);
        void* dst = pick_buffer(0, total_compressors, remaining_compressors, src_adj, dest, buff_tmp1, buff_tmp2);

        switch (datatype) {
            case (SCIL_TYPE_FLOAT):
                ret = algo->c.Ctype.decompress_float(dst, dims, src, src_size);
                break;
            case (SCIL_TYPE_DOUBLE):
                ret = algo->c.Ctype.decompress_double(dst, dims, src, src_size);
                break;
			case (SCIL_TYPE_INT8) :
				ret = algo->c.Ctype.decompress_int8(dst, dims, src, src_size);
				break;
			case(SCIL_TYPE_INT16) :
				ret = algo->c.Ctype.decompress_int16(dst, dims, src, src_size);
				break;
			case(SCIL_TYPE_INT32) :
				ret = algo->c.Ctype.decompress_int32(dst, dims, src, src_size);
				break;
			case(SCIL_TYPE_INT64) :
				ret = algo->c.Ctype.decompress_int64(dst, dims, src, src_size);
				break;
			case(SCIL_TYPE_STRING) :
				assert(0);
				break;
        }

        if (ret != 0) return ret;
        remaining_compressors--;
        if (remaining_compressors > 0) {
            // scilU_print_buffer(dst, src_size);
            compressor_id = *((char*)header);
            header--;
            CHECK_COMPRESSOR_ID(compressor_id)
            algo = algo_array[compressor_id];
        }
    }

    // the last compressors must be preconditioners
    for (int i = remaining_compressors; i > 0; i--) {
        void* src = pick_buffer(1, total_compressors, remaining_compressors, src_adj, dest, buff_tmp1, buff_tmp2);
        void* dst = pick_buffer(0, total_compressors, remaining_compressors, src_adj, dest, buff_tmp1, buff_tmp2);
        int header_parsed;

        if (algo->type != SCIL_COMPRESSOR_TYPE_DATATYPES_PRECONDITIONER_FIRST) {
            return SCIL_BUFFER_ERR;
        }

        switch (datatype) {
            case (SCIL_TYPE_FLOAT):
                ret = algo->c.PFtype.decompress_float(dst, dims, src, header, &header_parsed);
                break;
            case (SCIL_TYPE_DOUBLE):
                ret = algo->c.PFtype.decompress_double(dst, dims, src, header, &header_parsed);
                break;
			case (SCIL_TYPE_INT8) :
				ret = algo->c.PFtype.decompress_int8(dst, dims, src, header, &header_parsed);
				break;
			case(SCIL_TYPE_INT16) :
				ret = algo->c.PFtype.decompress_int16(dst, dims, src, header, &header_parsed);
				break;
			case(SCIL_TYPE_INT32) :
				ret = algo->c.PFtype.decompress_int32(dst, dims, src, header, &header_parsed);
				break;
			case(SCIL_TYPE_INT64) :
				ret = algo->c.PFtype.decompress_int64(dst, dims, src, header, &header_parsed);
				break;
			case(SCIL_TYPE_STRING) :
				assert(0);
				break;
        }
        header -= header_parsed;

        if (ret != 0) return ret;
        remaining_compressors--;

        if (remaining_compressors > 0) {
            // scilU_print_buffer(dst, src_size);
            compressor_id = *((char*)header);
            debugI("D compressor ID %d at pos %llu\n",
                   compressor_id,
                   (long long unsigned)header);
            header--;
            CHECK_COMPRESSOR_ID(compressor_id)
            algo = algo_array[compressor_id];
        }
    }
    // TODO check if the header is completely devoured.

    return SCIL_NO_ERR;
}

void scil_determine_accuracy(enum SCIL_Datatype datatype,
                             const void* restrict data_1,
                             const void* restrict data_2,
                             scil_dims_t* dims,
                             const double relative_err_finest_abs_tolerance,
                             scil_user_hints_t* out_hints) {
    scil_user_hints_t a;
    scilPr_initialize_user_hints(&a);

    a.absolute_tolerance                = 0;
    a.relative_err_finest_abs_tolerance = 0;
    a.relative_tolerance_percent        = 0;

		switch(datatype){
    case (SCIL_TYPE_DOUBLE): {
        a.significant_bits = MANTISSA_LENGTH_DOUBLE; // in bits
        scil_determine_accuracy_double((double*)data_1,
                                       (double*)data_2,
                                       scilPr_get_dims_count(dims),
                                       relative_err_finest_abs_tolerance,
                                       &a);
				break;
    }
		case (SCIL_TYPE_FLOAT) : {
        a.significant_bits = MANTISSA_LENGTH_FLOAT; // in bits
        scil_determine_accuracy_float((float*)data_1,
                                      (float*)data_2,
                                      scilPr_get_dims_count(dims),
                                      relative_err_finest_abs_tolerance,
                                      &a);
    		break;
		}
		case(SCIL_TYPE_INT8):{
			a.significant_bits = 8;
			scil_determine_accuracy_int8((int8*)data_1,
															(int8*)data_2,
															scilPr_get_dims_count(dims),
															relative_err_finest_abs_tolerance,
															&a);
			break;
		}
		case(SCIL_TYPE_INT16):{
			a.significant_bits = 16;
			scil_determine_accuracy_int16((int16*)data_1, (int16*)data_2, scilPr_get_dims_count(dims), relative_err_finest_abs_tolerance, &a);
			break;
		}
		case(SCIL_TYPE_INT32):{
			a.significant_bits = 32;
			scil_determine_accuracy_int32((int32*)data_1, (int32*)data_2, scilPr_get_dims_count(dims), relative_err_finest_abs_tolerance, &a);
			break;
		}
		case(SCIL_TYPE_INT64):{
			a.significant_bits = 64;
			scil_determine_accuracy_int64((int64*)data_1, (int64*)data_2, scilPr_get_dims_count(dims), relative_err_finest_abs_tolerance, &a);
			break;
		}
		case(SCIL_TYPE_STRING):{
			// No relevant comparision
			*out_hints = a;
			return;
		}
		}

    // convert significant_digits in bits to 10 decimals
    a.significant_digits =     scilU_convert_significant_bits_to_decimals(a.significant_bits);
    a.relative_tolerance_percent *= 100.0;

    if (a.relative_err_finest_abs_tolerance == 0) {
        a.relative_err_finest_abs_tolerance = a.absolute_tolerance;
    }

    *out_hints = a;
}

int scil_validate_compression(enum SCIL_Datatype datatype,
                              const void* restrict data_uncompressed,
                              scil_dims_t* dims,
                              byte* restrict data_compressed,
                              const size_t compressed_size,
                              const scil_context_t* ctx,
                              scil_user_hints_t* out_accuracy) {
    const uint64_t length = scilPr_get_compressed_data_size_limit(dims, datatype);
    byte* data_out        = (byte*)malloc(length);
    if (data_out == NULL) {
        return SCIL_MEMORY_ERR;
    }
    scil_user_hints_t a;

    memset(data_out, -1, length);

    int ret = scil_decompress(datatype,
                              data_out,
                              dims,
                              data_compressed,
                              compressed_size,
                              &data_out[length / 2]);
    if (ret != 0) {
        goto end;
    }

    if (ctx->lossless_compression_needed) {
        // check bytes for identity
        ret = memcmp(data_out, (byte*)data_uncompressed, length);
        memset(&a, 0, sizeof(a));

        if (!ctx->lossless_compression_needed) {
            printf(
                "INFO: can check only for identical data as data is not a "
                "multiple of DataType\n");
        } else {
            scil_determine_accuracy(
                datatype,
                data_out,
                data_uncompressed,
                dims,
                ctx->hints.relative_err_finest_abs_tolerance,
                &a);
        }
        goto end;
    } else {
        // determine achieved accuracy
        scil_determine_accuracy(datatype,
                                data_out,
                                data_uncompressed,
                                dims,
                                ctx->hints.relative_err_finest_abs_tolerance,
                                &a);

        const scil_user_hints_t h = ctx->hints;

        // check if tolerance level is met:
        ret = 0;
        if (a.absolute_tolerance > h.absolute_tolerance) {
            debug("Validation error absolute_tolerance %f > %f\n",
                  a.absolute_tolerance,
                  h.absolute_tolerance);
            ret = 1;
        }
        if (a.relative_tolerance_percent > h.relative_tolerance_percent) {
            debug("Validation error relative_tolerance_percent %f > %f\n",
                  a.relative_tolerance_percent,
                  h.relative_tolerance_percent);
            ret = 1;
        }
        if (a.relative_err_finest_abs_tolerance >
            h.relative_err_finest_abs_tolerance) {
            debug(
                "Validation error relative_err_finest_abs_tolerance %f < %f\n",
                a.relative_err_finest_abs_tolerance,
                h.relative_err_finest_abs_tolerance);
            ret = 1;
        }
        if (a.significant_digits < h.significant_digits) {
            debug("Validation error significant_digits %d > %d\n",
                  a.significant_digits,
                  h.significant_digits);
            ret = 1;
        }
        if (a.significant_bits < h.significant_bits) {
            debug("Validation error significant_bits %d > %d\n",
                  a.significant_bits,
                  h.significant_bits);
            ret = 1;
        }
    }

end:
    free(data_out);
    *out_accuracy = a;

    return ret;
}
