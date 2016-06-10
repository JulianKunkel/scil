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

#include <ctype.h>
#include <float.h>
#include <math.h>
#include <string.h>

#include <scil-algo-chooser.h>
#include <scil-hardware-limits.h>
#include <scil-internal.h>

// known algorithms:
#include <algo/algo-abstol.h>
#include <algo/algo-fpzip.h>
#include <algo/algo-gzip.h>
#include <algo/algo-memcopy.h>
#include <algo/algo-sigbits.h>
#include <algo/algo-zfp-abstol.h>
#include <algo/algo-zfp-precision.h>
#include <algo/algo-opj.h>
#include <algo/lz4fast.h>
#include <algo/precond-dummy.h>

// this file is automatically created
#include "scil-dtypes.h"

static scil_compression_algorithm * algo_array[] = {
	& algo_memcopy,
	& algo_abstol,
	& algo_gzip,
	& algo_sigbits,
	& algo_fpzip,
	& algo_zfp_abstol,
	& algo_zfp_precision,
	& algo_lz4fast,
	& algo_precond_dummy,
	& algo_opj,
	NULL
};

static char * performance_units[] = {
	"IGNORE",
	"MiB",
	"GiB",
	"NetworkSpeed",
	"NodeLocalStorageSpeed",
	"SingleStreamSharedStorageSpeed"
};


int scil_compressors_available(){
	static int count = -1;
	if (count > 0){
		return count;
	}

	scil_compression_algorithm ** cur = algo_array;
	count = 0;
	// count
	while(*cur != NULL){
		count++;
		cur++;
	}

	return count;
}

const char* scil_compressor_name(int num)
{
    assert(num < scil_compressors_available());
    return algo_array[num]->name;
}

void scil_compression_sprint_last_algorithm_chain(scil_context_p ctx,
                                                  char* out,
                                                  int buff_length)
{
    int ret                      = 0;
    scil_compression_chain_t* lc = &ctx->chain;
    for (int i = 0; i < PRECONDITIONER_LIMIT; i++) {
        if (lc->pre_cond[i] == NULL) break;
        ret = snprintf(out, buff_length, "%s,", lc->pre_cond[i]->name);
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

int scil_compressor_num_by_name(const char* name)
{
    scil_compression_algorithm** cur = algo_array;
    int count                        = 0;

    // check if this is a number
    int num = 1;
    for (unsigned i = 0; i < strlen(name); i++) {
        if (!isdigit(name[i])) {
            num = 0;
            break;
        }
    }
    if (num) {
        return atoi(name);
    }

    // count
    while (*cur != NULL) {
        if (strcmp((*cur)->name, name) == 0) {
            return count;
        }
        count++;
        cur++;
    }

    return -1;
}

#pragma GCC diagnostic ignored "-Wfloat-equal"
static int check_compress_lossless_needed(scil_context_p ctx)
{
    const scil_hints hints = ctx->hints;

    if ((hints.absolute_tolerance != SCIL_ACCURACY_DBL_IGNORE &&
         hints.absolute_tolerance <= SCIL_ACCURACY_DBL_FINEST) ||
        (hints.relative_err_finest_abs_tolerance <= SCIL_ACCURACY_DBL_FINEST &&
         hints.relative_err_finest_abs_tolerance != SCIL_ACCURACY_DBL_IGNORE) ||
        (hints.relative_tolerance_percent <= SCIL_ACCURACY_DBL_FINEST &&
         hints.relative_tolerance_percent != SCIL_ACCURACY_DBL_IGNORE) ||
        (hints.significant_digits == SCIL_ACCURACY_INT_FINEST) ||
        (hints.significant_bits == SCIL_ACCURACY_INT_FINEST)) {
        return 1;
    }
    return 0;
}

// This function is useful as it changes the sensitivity of the maximum double
// value
static void fix_double_setting(double* dbl)
{
    if (*dbl == SCIL_ACCURACY_DBL_IGNORE) {
        *dbl = DBL_MAX;
    }
}

scil_compression_algorithm* scilI_find_compressor_by_name(const char* name)
{
    int num = scil_compressor_num_by_name(name);
    if (num < 0 || num >= scil_compressors_available()) {
        return NULL;
    } else {
        return algo_array[num];
    }
}

int scilI_parse_compression_algorithms(scil_compression_chain_t* chain,
                                       char* str_in)
{
    char *saveptr, *token;
    char str[4096];
    strncpy(str, str_in, 4096);
    token = strtok_r(str, ",", &saveptr);

    int stage            = 1; // pre-conditioner
    chain->precond_count = 0;
    chain->total_size    = 0;

    char lossy = 0;
    for (int i = 0; token != NULL; i++) {
        scil_compression_algorithm* algo = scilI_find_compressor_by_name(token);
        if (algo == NULL) {
            return SCIL_EINVAL;
        }
        chain->total_size++;
        lossy += algo->is_lossy;
        switch (algo->type) {
            case (SCIL_COMPRESSOR_TYPE_DATATYPES_PRECONDITIONER): {
                if (stage != 1) {
                    return -1; // INVALID CHAIN 
                }
                chain->pre_cond[(int)chain->precond_count] = algo;
                chain->precond_count++;
                break;
            }
            case (SCIL_COMPRESSOR_TYPE_INDIVIDUAL_BYTES): {
                if (stage > 2) {
                    return -1; // INVALID CHAIN
                }
                stage                  = 3;
                chain->byte_compressor = algo;
                break;
            }
            case (SCIL_COMPRESSOR_TYPE_DATATYPES): {
                if (stage != 1) {
                    return -1; // INVALID CHAIN
                }
                stage                  = 2;
                chain->data_compressor = algo;
                break;
            }
        }
        token = strtok_r(NULL, ",", &saveptr);
    }
    chain->is_lossy = lossy > 0 ? 1 : 0;

    // at least one algo should be set
    if (chain->total_size == 0) {
        return SCIL_EINVAL;
    }

    return SCIL_NO_ERR;
}

void scil_init_dims_1d(scil_dims* dims, size_t dim1)
{
    dims->dims      = 1;
    dims->length[0] = dim1;
}
void scil_init_dims_2d(scil_dims* dims, size_t dim1, size_t dim2)
{
    dims->dims      = 2;
    dims->length[0] = dim1;
    dims->length[1] = dim2;
}

void scil_init_dims_3d(scil_dims* dims, size_t dim1, size_t dim2, size_t dim3){
	dims->dims = 3;
	dims->length[0] = dim1;
	dims->length[1] = dim2;
	dims->length[2] = dim3;
}

void scil_init_dims_4d(scil_dims* dims, size_t dim1, size_t dim2, size_t dim3, size_t dim4){
	dims->dims = 4;
	dims->length[0] = dim1;
	dims->length[1] = dim2;
	dims->length[2] = dim3;
	dims->length[3] = dim4;
}

void scil_copy_dims_array(scil_dims* out_dims, scil_dims in_dims){
	out_dims->dims = in_dims.dims;
	memcpy(out_dims->length, & in_dims.length, sizeof(size_t)* in_dims.dims);
}

void scil_init_dims_array(scil_dims* dims, uint8_t count, const size_t* length)
{
    dims->dims = count;
    assert(count <= 4);
    memcpy(&dims->length, length, count * sizeof(size_t));
}

size_t scil_get_data_count(const scil_dims* dims)
{
    size_t result = 1;
    for (uint8_t i = 0; i < dims->dims; ++i) {
        result *= dims->length[i];
    }
    return result;
}

size_t scil_get_data_size(enum SCIL_Datatype datatype, const scil_dims* dims)
{
    if (dims->dims == 0) {
        return 0;
    }
    size_t result = 1;
    for (uint8_t i = 0; i < dims->dims; ++i) {
        result *= dims->length[i];
    }
    return result * DATATYPE_LENGTH(datatype);
}

void scil_init_hints(scil_hints* hints)
{
    memset(hints, 0, sizeof(scil_hints));
    hints->relative_tolerance_percent        = SCIL_ACCURACY_DBL_IGNORE;
    hints->relative_err_finest_abs_tolerance = SCIL_ACCURACY_DBL_IGNORE;
    hints->absolute_tolerance                = SCIL_ACCURACY_DBL_IGNORE;

    hints->comp_speed.unit   = SCIL_PERFORMANCE_IGNORE;
    hints->decomp_speed.unit = SCIL_PERFORMANCE_IGNORE;
}

static void print_performance_hint(const char* name,
                                   const scil_performance_hint_t p)
{
    printf("\t%s: %f * %s\n",
           name,
           (double)p.multiplier,
           performance_units[p.unit]);
}

void scil_hints_print(const scil_hints* h)
{
    printf(
        "Precision hints: \n\trelative_tolerance_percent: %g "
        "\n\trelative_err_finest_abs_tolerance: %g\n\tabsolute_tolerance: %g "
        "\n\tsignificant_digits: %d \n\tsignificant_bits (in the mantissa): "
        "%d\n",
        h->relative_tolerance_percent,
        h->relative_err_finest_abs_tolerance,
        h->absolute_tolerance,
        h->significant_digits,
        h->significant_bits);
    printf("Performance hints:\n");
    print_performance_hint("Compression", h->comp_speed);
    print_performance_hint("Decompression", h->decomp_speed);
}

int scil_destroy_compression_context(scil_context_p* out_ctx)
{
    scil_hints h;
    h = (*out_ctx)->hints;

    if (h.force_compression_methods != NULL) {
        free(h.force_compression_methods);
    }

    free(*out_ctx);
    *out_ctx = NULL;

    return SCIL_NO_ERR;
}

scil_hints scil_retrieve_effective_hints(scil_context_p ctx)
{
    return ctx->hints;
}

static int scil_initialized = 0;

static void scil_check_if_initialized()
{
    if (scil_initialized) {
        return;
    }

    // verify correctness of algo_array
    int i = 0;
    for (scil_compression_algorithm **algo = algo_array; *algo != NULL;
         algo++, i++) {
        if ((*algo)->magic_number != i) {
            scilU_critical_error("Magic number does not match!");
        }
        if ((*algo)->type == SCIL_COMPRESSOR_TYPE_INDIVIDUAL_BYTES) {
            // we expect that all byte compressors are lossless
            (*algo)->is_lossy = 0;
        }
    }

    scilI_init_hardware_limits();
    scilI_compression_algo_chooser_init();
    scil_initialized = 1;
}

int scil_create_compression_context(scil_context_p* out_ctx,
                                    enum SCIL_Datatype datatype,
                                    const scil_hints* hints)
{
    scil_check_if_initialized();

    int ret = SCIL_NO_ERR;
    scil_context_p ctx;
    scil_hints* oh;

    *out_ctx = NULL;

    ctx = (scil_context_p)SAFE_MALLOC(sizeof(struct scil_context_t));
    memset(&ctx->chain, 0, sizeof(ctx->chain));
    ctx->datatype = datatype;

    oh = &ctx->hints;
    memcpy(oh, hints, sizeof(scil_hints));

    // adjust accuracy needed
    if (datatype == SCIL_TYPE_FLOAT) {
        if ((oh->significant_digits > 6) || (oh->significant_bits > 23)) {
            oh->significant_digits = SCIL_ACCURACY_INT_FINEST;
            oh->significant_bits   = SCIL_ACCURACY_INT_FINEST;
        }
    } else {
        if ((oh->significant_digits > 15) || (oh->significant_bits > 52)) {
            oh->significant_digits = SCIL_ACCURACY_INT_FINEST;
            oh->significant_bits   = SCIL_ACCURACY_INT_FINEST;
        }
    }

    // We convert between significant digits and bits and take the finer
    // granularity
    // An algorithm can then choose which of those metrics to use.
    if (oh->significant_digits != SCIL_ACCURACY_INT_IGNORE) {
        oh->significant_bits = max(
            oh->significant_bits,
            scilU_convert_significant_decimals_to_bits(oh->significant_digits));
    }

    // Why should this make any sense:
    // if(oh->relative_tolerance_percent != SCIL_ACCURACY_DBL_IGNORE){
    //	oh->significant_bits = max(oh->significant_bits,
    //scilU_relative_tolerance_to_significant_bits(oh->relative_tolerance_percent));
    //}

    if (oh->significant_bits != SCIL_ACCURACY_INT_IGNORE) {
        if (oh->significant_digits == SCIL_ACCURACY_INT_IGNORE) {
            oh->significant_digits = scilU_convert_significant_bits_to_decimals(oh->significant_bits);

            // we need to round the bits properly to decimals, i.e., 1 bit
            // precision in the mantissa requires 1 decimal digit.
            const int newbits = scilU_convert_significant_decimals_to_bits(oh->significant_digits);
            if (newbits < oh->significant_bits) {
                oh->significant_digits = scilU_convert_significant_bits_to_decimals(oh->significant_bits);
            }
        } else {
            oh->significant_digits = max(scilU_convert_significant_bits_to_decimals(oh->significant_bits), oh->significant_digits);
        }

        // Why should this make any sense:
        // if(oh->relative_tolerance_percent == SCIL_ACCURACY_DBL_IGNORE){
        //	oh->relative_tolerance_percent =
        //scilU_significant_bits_to_relative_tolerance(oh->significant_bits);
        //}
    }

    ctx->lossless_compression_needed = check_compress_lossless_needed(ctx);
    fix_double_setting(&oh->relative_tolerance_percent);
    fix_double_setting(&oh->relative_err_finest_abs_tolerance);
    fix_double_setting(&oh->absolute_tolerance);
    // TODO handle float differently.

    if (oh->force_compression_methods != NULL) {
        // now we can prefill the compression pipeline
        ret = scilI_parse_compression_algorithms(&ctx->chain, hints->force_compression_methods);

        oh->force_compression_methods = strdup(oh->force_compression_methods);
    }

    if (ret == SCIL_NO_ERR) {
        *out_ctx = ctx;
    } else {
        free(ctx);
    }

    return ret;
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

size_t scil_compress_buffer_size_bound(enum SCIL_Datatype datatype,
                                       const scil_dims* dims)
{
    return scil_get_data_size(datatype, dims) * 4 + SCIL_BLOCK_HEADER_MAX_SIZE;
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
- byte magic_number // The compressor number as registered in SCIL.
- byte * COMPRESSOR_SPECIFIC_HEADER
- byte * COMPRESSED DATA

If the chain consists of multiple compressors (n-many) the final format looks
like:

byte CHAIN_LENGTH
byte magic_number_ALGO(n)
ALGO(n) specific header
ALGO(n-1) data compressed using ALGO(n)

If ALGO(n-1) is a datatype specific algorithm, then it usually cannot handle
arbitrary bytes.
Therefore, the magic number and headers of nested datatypes must be split from
the data.

A datatype compressor terminates the chain of preconditioners.
 */
int scil_compress(byte* restrict dest,
                  size_t in_dest_size,
                  void* restrict source,
                  scil_dims* dims,
                  size_t* restrict out_size_p,
                  scil_context_p ctx)
{
    int ret = SCIL_NO_ERR;

    assert(ctx != NULL);
    assert(dest != NULL);
    assert(out_size_p != NULL);
    assert(source != NULL);

    size_t input_size           = scil_get_data_size(ctx->datatype, dims);
    const size_t datatypes_size = input_size;

    if (datatypes_size == 0) {
        *out_size_p = 1;
        dest[0]     = (byte)0;

        return SCIL_NO_ERR;
    }
    if (in_dest_size < 4 * datatypes_size) {
        return SCIL_MEMORY_ERR;
    }

    const scil_hints* hints         = &ctx->hints;
    scil_compression_chain_t* chain = &ctx->chain;

    if (hints->force_compression_methods == NULL) { // if != NULL do nothing as we have parsed the pipeline already
        scilI_compression_algo_chooser(source, dims, ctx);
    }

    size_t out_size = 0;

    // Add the length of the algo chain to the output
    int remaining_compressors   = chain->total_size;
    const int total_compressors = remaining_compressors;
    dest[0]                     = total_compressors;
    dest++;

    // now process the compression pipeline
    // we use 1.5 the memory buffer as intermediate location
    const size_t buffer_tmp_offset = 2 * datatypes_size + 10;
    byte* restrict buff_tmp        = &dest[buffer_tmp_offset];

    // process the compression chain
    // apply the pre-conditioners
    if (chain->precond_count > 0) {
        out_size += datatypes_size;
        // add the header at the end of the preconditioners
        byte* header =
            datatypes_size +
            (byte*)pick_buffer(0,
                               total_compressors,
                               1 + total_compressors - chain->precond_count,
                               source,
                               dest,
                               buff_tmp,
                               dest);

        for (int i = 0; i < chain->precond_count; i++) {
            int header_size_out;
            scil_compression_algorithm* algo = chain->pre_cond[i];
            void* src                        = pick_buffer(1,
                                    total_compressors,
                                    remaining_compressors,
                                    source,
                                    dest,
                                    buff_tmp,
                                    dest);
            void* dst = pick_buffer(0,
                                    total_compressors,
                                    remaining_compressors,
                                    source,
                                    dest,
                                    buff_tmp,
                                    dest);

            switch (ctx->datatype) {
                case (SCIL_TYPE_FLOAT):
                    ret = algo->c.Ptype.compress_float(
                        ctx, (float*)dst, header, &header_size_out, src, dims);
                    break;
                case (SCIL_TYPE_DOUBLE):
                    ret = algo->c.Ptype.compress_double(
                        ctx, (double*)dst, header, &header_size_out, src, dims);
                    break;
            }

            if (ret != 0) return ret;
            remaining_compressors--;
            out_size += header_size_out;
            header += header_size_out;

            *header = algo->magic_number;
            debugI(
                "C MAGIC %d at pos %llu\n", *header, (long long unsigned)header)
                header++;
            out_size++;

            // scilU_print_buffer(dst, out_size);
        }
        input_size = out_size;
    }

    if (chain->data_compressor) {
        // we need to preserve the header of the pre-conditioners.
        void* src = pick_buffer(1,
                                total_compressors,
                                remaining_compressors,
                                source,
                                dest,
                                buff_tmp,
                                dest);
        void* dst = pick_buffer(0,
                                total_compressors,
                                remaining_compressors,
                                source,
                                dest,
                                buff_tmp,
                                dest);

        // set the output size to the expected buffer size
        out_size = (size_t)(datatypes_size * 2);

        scil_compression_algorithm* algo = chain->data_compressor;
        switch (ctx->datatype) {
            case (SCIL_TYPE_FLOAT):
                ret = algo->c.DNtype.compress_float(
                    ctx, dst, &out_size, src, dims);
                break;
            case (SCIL_TYPE_DOUBLE):
                ret = algo->c.DNtype.compress_double(
                    ctx, dst, &out_size, src, dims);
                break;
        }
        if (ret != 0) return ret;
        // check if we have to preserve another header from the preconditioners
        if (datatypes_size != input_size) {
            // we have to copy some header.
            debugI("Preserving %lld %lld\n",
                   (long long)datatypes_size,
                   (long long)input_size);
            const int preserve = input_size - datatypes_size;
            memcpy(
                (char*)dst + out_size, (char*)src + datatypes_size, preserve);
            out_size += preserve;
            scilU_print_buffer(dst, out_size);
        }

        remaining_compressors--;
        ((char*)dst)[out_size] = algo->magic_number;
        debugI("C MAGIC %d at pos %llu\n",
               algo->magic_number,
               (long long unsigned)&((char*)dst)[out_size]);

        out_size++;
        input_size = out_size;
    }

    if (chain->byte_compressor) {
        void* src = pick_buffer(1,
                                total_compressors,
                                remaining_compressors,
                                source,
                                dest,
                                buff_tmp,
                                dest);

        // scilU_print_buffer(src, input_size);

        ret = chain->byte_compressor->c.Btype.compress(
            ctx, dest, &out_size, (byte*)src, input_size);
        if (ret != 0) return ret;
        dest[out_size] = chain->byte_compressor->magic_number;
        debugI("C MAGIC %d at pos %llu\n",
               chain->byte_compressor->magic_number,
               (long long unsigned)&dest[out_size]);

        out_size++;
        // scilU_print_buffer(dest, out_size);
    }

    *out_size_p = out_size + 1; // for the length of the processing chain
    return SCIL_NO_ERR;
}

#define CHECK_MAGIC(magic)                              \
    if (magic_number >= scil_compressors_available()) { \
        return SCIL_BUFFER_ERR;                         \
    }

int scil_decompress(enum SCIL_Datatype datatype,
                    void* restrict dest,
                    scil_dims* const dims,
                    byte* restrict source,
                    const size_t source_size,
                    byte* restrict buff_tmp1)
{
    if (dims->dims == 0) {
        return 0;
    }

    assert(dest != NULL);
    assert(source != NULL);
    assert(buff_tmp1 != NULL);

    // Read magic number (algorithm id) from header
    const int total_compressors = (uint8_t)source[0];
    int remaining_compressors   = total_compressors;

    byte* restrict src_adj = source + 1;
    size_t src_size        = source_size - 1;
    int ret;

    const size_t output_size = scil_get_data_size(datatype, dims);
    byte* restrict buff_tmp2 = &buff_tmp1[(int)(output_size * 2 + 10)];

    // for(int i=0; i < chain_size; i++){
    src_size--;
    uint8_t magic_number = src_adj[src_size];

    debugI("D MAGIC %d at pos %llu\n",
           magic_number,
           (long long unsigned)&src_adj[src_size]);

    CHECK_MAGIC(magic_number)
    // printf("SCHUH %d %lld\n", magic_number, source_size);

    scil_compression_algorithm* algo = algo_array[magic_number];
    byte* header                     = &src_adj[src_size - 1];

    if (algo->type == SCIL_COMPRESSOR_TYPE_INDIVIDUAL_BYTES) {
        void* src = pick_buffer(1,
                                total_compressors,
                                remaining_compressors,
                                src_adj,
                                dest,
                                buff_tmp1,
                                buff_tmp2);
        void* dst = pick_buffer(0,
                                total_compressors,
                                remaining_compressors,
                                src_adj,
                                dest,
                                buff_tmp1,
                                buff_tmp2);

        ret = algo->c.Btype.decompress(
            dst, output_size * 2 + 10, (byte*)src, src_size, &src_size);
        if (ret != 0) return ret;
        remaining_compressors--;

        // the header is on the right hand side of the buffer
        header = &((byte*)dst)[src_size];

        if (remaining_compressors > 0) {
            // scilU_print_buffer(dst, src_size);

            header--;
            magic_number = header[0];
            header--;

            debugI("D MAGIC %d at pos %llu\n",
                   magic_number,
                   (long long unsigned)header);
            CHECK_MAGIC(magic_number)
            algo = algo_array[magic_number];
        }
    }

    if (algo->type == SCIL_COMPRESSOR_TYPE_DATATYPES) {
        void* src = pick_buffer(1,
                                total_compressors,
                                remaining_compressors,
                                src_adj,
                                dest,
                                buff_tmp1,
                                buff_tmp2);
        void* dst = pick_buffer(0,
                                total_compressors,
                                remaining_compressors,
                                src_adj,
                                dest,
                                buff_tmp1,
                                buff_tmp2);

        switch (datatype) {
            case (SCIL_TYPE_FLOAT):
                ret = algo->c.DNtype.decompress_float(dst, dims, src, src_size);
                break;
            case (SCIL_TYPE_DOUBLE):
                ret =
                    algo->c.DNtype.decompress_double(dst, dims, src, src_size);
                break;
        }

        if (ret != 0) return ret;
        remaining_compressors--;
        if (remaining_compressors > 0) {
            // scilU_print_buffer(dst, src_size);
            magic_number = *((char*)header);
            header--;
            CHECK_MAGIC(magic_number)
            algo = algo_array[magic_number];
        }
    }

    // the last compressors must be preconditioners
    for (int i = remaining_compressors; i > 0; i--) {
        void* src = pick_buffer(1,
                                total_compressors,
                                remaining_compressors,
                                src_adj,
                                dest,
                                buff_tmp1,
                                buff_tmp2);
        void* dst = pick_buffer(0,
                                total_compressors,
                                remaining_compressors,
                                src_adj,
                                dest,
                                buff_tmp1,
                                buff_tmp2);
        int header_parsed;

        if (algo->type != SCIL_COMPRESSOR_TYPE_DATATYPES_PRECONDITIONER) {
            return SCIL_BUFFER_ERR;
        }

        switch (datatype) {
            case (SCIL_TYPE_FLOAT):
                ret = algo->c.Ptype.decompress_float(
                    dst, dims, src, header, &header_parsed);
                break;
            case (SCIL_TYPE_DOUBLE):
                ret = algo->c.Ptype.decompress_double(
                    dst, dims, src, header, &header_parsed);
                break;
        }
        header -= header_parsed;

        if (ret != 0) return ret;
        remaining_compressors--;

        if (remaining_compressors > 0) {
            // scilU_print_buffer(dst, src_size);
            magic_number = *((char*)header);
            debugI("D MAGIC %d at pos %llu\n",
                   magic_number,
                   (long long unsigned)header);
            header--;
            CHECK_MAGIC(magic_number)
            algo = algo_array[magic_number];
        }
    }
    // TODO check if the header is completely devoured.

    return SCIL_NO_ERR;
}

void scil_determine_accuracy(enum SCIL_Datatype datatype,
                             const void* restrict data_1,
                             const void* restrict data_2,
                             scil_dims* dims,
                             const double relative_err_finest_abs_tolerance,
                             scil_hints* out_hints)
{
    scil_hints a;
    scil_init_hints(&a);

    a.absolute_tolerance                = 0;
    a.relative_err_finest_abs_tolerance = 0;
    a.relative_tolerance_percent        = 0;

    if (datatype == SCIL_TYPE_DOUBLE) {
        a.significant_bits = MANTISSA_LENGTH_DOUBLE; // in bits
        scil_determine_accuracy_double((double*)data_1,
                                       (double*)data_2,
                                       scil_get_data_count(dims),
                                       relative_err_finest_abs_tolerance,
                                       &a);
    } else {
        a.significant_bits = MANTISSA_LENGTH_FLOAT; // in bits
        scil_determine_accuracy_float((float*)data_1,
                                      (float*)data_2,
                                      scil_get_data_count(dims),
                                      relative_err_finest_abs_tolerance,
                                      &a);
    }

    // convert significant_digits in bits to 10 decimals
    a.significant_digits =
        scilU_convert_significant_bits_to_decimals(a.significant_bits);
    a.relative_tolerance_percent *= 100.0;

    if (a.relative_err_finest_abs_tolerance == 0) {
        a.relative_err_finest_abs_tolerance = a.absolute_tolerance;
    }

    *out_hints = a;
}

int scil_validate_compression(enum SCIL_Datatype datatype,
                              const void* restrict data_uncompressed,
                              scil_dims* dims,
                              byte* restrict data_compressed,
                              const size_t compressed_size,
                              const scil_context_p ctx,
                              scil_hints* out_accuracy)
{
    const uint64_t length = scil_compress_buffer_size_bound(datatype, dims);
    byte* data_out        = (byte*)malloc(length);
    if (data_out == NULL) {
        return SCIL_MEMORY_ERR;
    }
    scil_hints a;

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

        const scil_hints h = ctx->hints;

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
