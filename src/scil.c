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
#include <float.h>
#include <math.h>
#include <ctype.h>

#include <scil-internal.h>

// known algorithms:
#include <algo/algo-memcopy.h>
#include <algo/algo-abstol.h>
#include <algo/algo-gzip.h>
#include <algo/algo-sigbits.h>
#include <algo/algo-fpzip.h>
#include <algo/algo-zfp-abstol.h>
#include <algo/algo-zfp-precision.h>
#include <algo/lz4fast.h>

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

const char * scil_compressor_name(int num){
	assert(num < scil_compressors_available());
	return algo_array[num]->name;
}

int scil_compressor_num_by_name(const char * name){
	scil_compression_algorithm ** cur = algo_array;
	int count = 0;

	// check if this is a number
	int num = 1;
	for(unsigned i=0; i < strlen(name); i++){
		if (! isdigit(name[i])){
			num = 0;
			break;
		}
	}
	if (num){
		return atoi(name);
	}

	// count
	while(*cur != NULL){
		if (strcmp((*cur)->name, name) == 0 ){
			return count;
		}
		count++;
		cur++;
	}

	return -1;
}

#pragma GCC diagnostic ignored "-Wfloat-equal"
static int check_compress_lossless_needed(scil_context_p ctx){
	const scil_hints hints = ctx->hints;

	if ( (hints.absolute_tolerance != SCIL_ACCURACY_DBL_IGNORE && hints.absolute_tolerance <= SCIL_ACCURACY_DBL_FINEST)
		|| (hints.relative_err_finest_abs_tolerance <= SCIL_ACCURACY_DBL_FINEST && hints.relative_err_finest_abs_tolerance != SCIL_ACCURACY_DBL_IGNORE)
		|| (hints.relative_tolerance_percent <= SCIL_ACCURACY_DBL_FINEST && hints.relative_tolerance_percent != SCIL_ACCURACY_DBL_IGNORE)
		|| ( hints.significant_digits != SCIL_ACCURACY_INT_IGNORE &&
			(hints.significant_digits >= SCIL_ACCURACY_SIGNIFICANT_FINEST || hints.significant_digits == SCIL_ACCURACY_INT_FINEST)) ){
		return 1;
	}
	return 0;
}

// This function is useful as it changes the sensitivity of the maximum double value
static void fix_double_setting(double * dbl){
	if(*dbl == SCIL_ACCURACY_DBL_IGNORE){
		*dbl = DBL_MAX;
	}
}

static scil_compression_algorithm * find_compressor_by_name(const char * name){
	int num = scil_compressor_num_by_name(name);
	if (num < 0 || num >= scil_compressors_available()){
		return NULL;
	}else{
		return algo_array[num];
	}
}

static int parse_compression_algorithms(scil_compression_chain_t * chain, char * str_in){
	char * saveptr, * token;
	char str[4096];
	strncpy(str, str_in, 4096);
	token = strtok_r(str, ",", & saveptr);
	chain->size = 0;

	int stage = 1; // pre-conditioner

	for( int i = 0; token != NULL; i++){
		scil_compression_algorithm * algo = find_compressor_by_name(token);
		if (algo == NULL){
			return -1; // INVALID ALGO
		}
		switch(algo->type){
			case(SCIL_COMPRESSOR_TYPE_DATATYPES_PRECONDITIONER):{
				if (stage != 1){
					return -1; // INVALID CHAIN
				}
				chain->pre_cond[chain->size] = algo;
				chain->size++;
				break;
			}case (SCIL_COMPRESSOR_TYPE_INDIVIDUAL_BYTES):{
				if (stage > 2){
					return -1; // INVALID CHAIN
				}
				stage = 3;
				chain->byte_compressor = algo;
				break;
			} case(SCIL_COMPRESSOR_TYPE_DATATYPES):{
				if (stage != 1){
					return -1; // INVALID CHAIN
				}
				stage = 2;
				chain->data_compressor = algo;
				break;
			}
		}
		token = strtok_r(NULL, ",", & saveptr);
	}
	chain->size = 0;

	return 0;
}


scil_dims_t scil_init_dims(const uint8_t dimensions_count, size_t* dimensions_length){
	return (scil_dims_t){ .dims = dimensions_count, .length = dimensions_length };
}

size_t scil_get_data_count(const scil_dims_t dims){

	size_t result = 1;
	for(uint8_t i = 0; i < dims.dims; ++i){
		result *= dims.length[i];
	}
	return result;
}

void scil_init_hints(scil_hints * hints){
	memset(hints, 0, sizeof(scil_hints));
	hints->relative_tolerance_percent = SCIL_ACCURACY_DBL_IGNORE;
  hints->relative_err_finest_abs_tolerance = SCIL_ACCURACY_DBL_IGNORE;
  hints->absolute_tolerance  = SCIL_ACCURACY_DBL_IGNORE;

	hints->comp_speed.unit = SCIL_PERFORMANCE_IGNORE;
	hints->decomp_speed.unit = SCIL_PERFORMANCE_IGNORE;
}

static void print_performance_hint(const char * name, scil_performance_hint_t p){
	printf("\t%s: %.4f * %s\n", name, p.multiplier, performance_units[p.unit]);
}

void scil_hints_print(scil_hints * h){
	printf("Precision hints: \n\trelative_tolerance_percent:%f \n\trelative_err_finest_abs_tolerance:%f \n\tabsolute_tolerance:%f \n\tsignificant_digits (after 1. so in the mantissa):%d \n\tsignificant_bits (in the mantissa):%d\n",
		h->relative_tolerance_percent, h->relative_err_finest_abs_tolerance, h->absolute_tolerance, h->significant_digits, h->significant_bits);
	printf("Performance hints:\n");
	print_performance_hint("Compression", h->comp_speed);
	print_performance_hint("Decompression", h->decomp_speed);
}

int scil_destroy_compression_context(scil_context_p * out_ctx){
	free(*out_ctx);
	*out_ctx = NULL;

	return SCIL_NO_ERR;
}

scil_hints scil_retrieve_effective_hints(scil_context_p ctx){
	return ctx->hints;
}

int scil_create_compression_context(scil_context_p * out_ctx, const scil_hints * hints){
	int ret = 0;
	scil_context_p ctx =(scil_context_p) SAFE_MALLOC(sizeof(struct scil_context_t));
	memset(&ctx->last_chain, 0, sizeof(ctx->last_chain));

	*out_ctx = NULL;
	scil_hints * ohints = & ctx->hints;

	memcpy(ohints, hints, sizeof(scil_hints));

	// convert between significant digits and bits
	if (ohints->significant_digits != SCIL_ACCURACY_INT_IGNORE){
		ohints->significant_bits = max(ohints->significant_bits, scilU_convert_significant_decimals_to_bits(ohints->significant_digits) );
	}

	if(ohints->relative_tolerance_percent != SCIL_ACCURACY_DBL_IGNORE){
		ohints->significant_bits = max(ohints->significant_bits, scilU_relative_tolerance_to_significant_bits(ohints->relative_tolerance_percent));
	}

	if (ohints->significant_bits != SCIL_ACCURACY_INT_IGNORE){
	 	if(ohints->significant_digits == SCIL_ACCURACY_INT_IGNORE){
			ohints->significant_digits = scilU_convert_significant_bits_to_decimals(ohints->significant_bits);

			// we need to round the bits properly to decimals, i.e., 1 bit precision in the mantissa requires 1 decimal digit.
			const int newbits = scilU_convert_significant_decimals_to_bits(ohints->significant_digits);
			if ( newbits < ohints->significant_bits ){
				ohints->significant_digits = scilU_convert_significant_bits_to_decimals(ohints->significant_bits) + 1;
			}
		}
		if(ohints->relative_tolerance_percent == SCIL_ACCURACY_DBL_IGNORE){
			ohints->relative_tolerance_percent = scilU_significant_bits_to_relative_tolerance(ohints->significant_bits);
		}
	}

	ctx->lossless_compression_needed = check_compress_lossless_needed(ctx);
	fix_double_setting(& ohints->relative_tolerance_percent);
	fix_double_setting(& ohints->relative_err_finest_abs_tolerance);
	fix_double_setting(& ohints->absolute_tolerance);
	ohints->significant_digits = (ohints->significant_digits == SCIL_ACCURACY_INT_IGNORE) ? 0 : ohints->significant_digits;

	// verify correctness of algo_array
	{
	int i = 0;
	for (scil_compression_algorithm ** algo = algo_array; *algo != NULL ; algo++, i++){
		if ((*algo)->magic_number != i){
			scilU_critical_error("Magic number does not match!");
		}
	}
	}

	if (hints->force_compression_methods != NULL){
		// now we can prefill the compression pipeline
		ret = parse_compression_algorithms(& ctx->last_chain, hints->force_compression_methods);
	}

	if (ret == 0){
		*out_ctx = ctx;
	}else{
		free(ctx);
	}

	return ret;
}

static inline void * pick_buffer(int is_src, int total, int remain, void*restrict src, void*restrict dest, void*restrict buff1, void*restrict buff2){
		if(remain == total && is_src){
			return src;
		}
		if (remain == 1 && ! is_src){
			return dest;
		}
		if (remain % 2 == is_src){
			return buff1;
		}else{
			return buff2;
		}
}

/*
A compression chain compresses data in multiple phases, i.e., applying algo 1, then algo 2 ...
The processing sequence may consist of the following steps:
1) a sequence of datatype preconditioners
2) a data compressor
3) a single byte compressor.
For cache efficiency reasons, a compound compression scheme should be used instead of multiple data copy stages.

Internally, the compressed buffer is formated as follows:
- byte CHAIN_LENGTH // the number of compressors to apply.

Then for the last compressor that has been applied the format looks like:
- byte magic_number // The compressor number as registered in SCIL.
- byte * COMPRESSOR_SPECIFIC_HEADER
- byte * COMPRESSED DATA

If the chain consists of multiple compressors (n-many) the final format looks like:

byte CHAIN_LENGTH
byte magic_number_ALGO(n)
ALGO(n) specific header
ALGO(n-1) data compressed using ALGO(n)

If ALGO(n-1) is a datatype specific algorithm, then it usually cannot handle arbitrary bytes.
Therefore, the magic number and headers of nested datatypes must be split from the data.

For illustration purpose, assume C == len of the chain, M is magic, H is header, D is data.
' indicates that a compression algorithm has been applied to the memory buffer, thus it will look like:
CHDM => C H' H D' M M' => C H'' H' H  D''  M M' M''

For decompression it depends on the type of the algorithm.
a) A byte compressor can (de-)compress everything except its magic and the count, so assume M'' is a byte compressor.
it will decompress everything in the brackets
C [H'' H' H  D''  M M'] M''

b) A datatype compressor, which cannot decompress headers and magics
C H'' H' H  [D'']  M M' M''
The location is determined by the number of remaining magic numbers; Since a datatype compressor is always the first to compress and last to decompress, the number of magic numbers that are comming is clear.

Compression works similarly, but we append C and M after compression has been finished.
a) A byte compressor
input: HDM of the previous stage, size of [HDM]
output: H'[HDM] ; M' is appended automatically.

b) A datatype preconditioner/compressor
input: D of the previous stage, dim.
output: H'[D] ; M' is appended automatically.

A datatype compressor terminates the chain of preconditioners.
 */
int scil_compress(enum SCIL_Datatype datatype, byte* restrict dest, size_t in_dest_size,
	void*restrict source, scil_dims_t dims, size_t* restrict out_size_p, scil_context_p ctx){
	int ret;

	if (dims.dims == 0){
		*out_size_p = 0;
		return 0;
	}
	size_t input_size = scil_get_data_count(dims) * DATATYPE_LENGTH(datatype);
	if (input_size == 0){
		*out_size_p = 0;
		return 0;
	}

	size_t out_size = 0;

	assert(ctx != NULL);
	assert(dest != NULL);
	assert(out_size_p != NULL);
	assert(source != NULL);

	const scil_hints * hints = & ctx->hints;
	scil_compression_chain_t * chain = & ctx->last_chain;

	if (hints->force_compression_methods == NULL){ // if != NULL do nothing as we have parsed the pipeline already
		if (ctx->lossless_compression_needed){
			// we cannot compress because data must be accurate!
			parse_compression_algorithms(chain, "memcpy");
		}else{
			// TODO: pick the best algorithm for the settings given in ctx...
			assert("No algorithm chooser available, yet");
		}
	}

	//Add the length of the algo chain to the output
	int remaining_compressors = chain->size + (chain->data_compressor != NULL ? 1 : 0) + (chain->byte_compressor != NULL ? 1 : 0);
	const int total_compressors = remaining_compressors;
	dest[0] = total_compressors;
	dest++;

	// now process the compression pipeline
	// we use 1.5 the memory buffer as intermediate location
	const size_t buffer_tmp_offset = (size_t) (1.5 * input_size)+10;
	byte* restrict buff_tmp = & dest[buffer_tmp_offset];

	// process the compression chain
	// apply the pre-conditioners
	for(int i=0; i < chain->size; i++){
		switch(datatype){
			case(SCIL_TYPE_FLOAT):
				//ret = algo->c.DPrecond.compress_float(ctx, dest, header_size_out, source, dims);
				break;
			case(SCIL_TYPE_DOUBLE):
				//ret = algo->c.DPrecond.compress_double(ctx, dest, header_size_out, source, dims);
				break;
		}

		if (ret != 0) return ret;
		remaining_compressors--;
	}

	if(chain->data_compressor){
		void * src = pick_buffer(1, total_compressors, remaining_compressors, source, dest, buff_tmp, dest);
		void * dst = pick_buffer(0, total_compressors, remaining_compressors, source, dest, buff_tmp, dest);

		scil_compression_algorithm * algo = chain->data_compressor;
		switch(datatype){
			case(SCIL_TYPE_FLOAT):
				ret = algo->c.DNtype.compress_float(ctx, dst, & out_size, src, dims);
				break;
			case(SCIL_TYPE_DOUBLE):
				ret = algo->c.DNtype.compress_double(ctx, dst, & out_size, src, dims);
				break;
		}
		if (ret != 0) return ret;
		remaining_compressors--;
		((char*)dst)[out_size] = algo->magic_number;
		out_size++;
		input_size = out_size;

		//scilU_print_buffer(dst, out_size);
	}

	if(chain->byte_compressor){
		void * src = pick_buffer(1, total_compressors, remaining_compressors, source, dest, buff_tmp, dest);

		ret = chain->byte_compressor->c.Btype.compress(ctx, dest, &out_size, (byte *) src, input_size);
		if (ret != 0) return ret;
		dest[out_size] = chain->byte_compressor->magic_number;
		out_size++;

		//printf("C %lld %d %d\n", out_size, chain->byte_compressor->magic_number, total_compressors);
		//scilU_print_buffer(dest, out_size);
	}

	*out_size_p = out_size + 1; // for the length of the processing chain
	return 0;
}

int scil_decompress(enum SCIL_Datatype datatype, void*restrict dest, scil_dims_t dims,
    byte*restrict source, const size_t source_size, byte*restrict buff_tmp1){

	//assert(dims.dims == 1);

	if (dims.dims == 0){
		return 0;
	}

	assert(dest != NULL);
	assert(source != NULL);

	// Read magic number (algorithm id) from header
	const int total_compressors = (uint8_t) source[0];
	int remaining_compressors = total_compressors;

	byte*restrict src_adj = source + 1;
	size_t src_size = source_size - 1;
	int ret;

	const size_t output_size = scil_get_data_count(dims) * DATATYPE_LENGTH(datatype);
	byte*restrict buff_tmp2 = & buff_tmp1[(int)(output_size*1.5+10)];

	//for(int i=0; i < chain_size; i++){
	uint8_t magic_number = src_adj[src_size-1];
	//printf("SCHUH %d %lld\n", magic_number, source_size);

	scil_compression_algorithm * algo = algo_array[magic_number];

	if(algo->type == SCIL_COMPRESSOR_TYPE_INDIVIDUAL_BYTES){
		void * src = pick_buffer(1, total_compressors, remaining_compressors, src_adj, dest, buff_tmp1, buff_tmp2);
		void * dst = pick_buffer(0, total_compressors, remaining_compressors, src_adj, dest, buff_tmp1, buff_tmp2);

		ret = algo->c.Btype.decompress(dst, (byte *) src, src_size, & src_size);
		if (ret != 0) return ret;
		remaining_compressors--;

		if(remaining_compressors > 0){
			//scilU_print_buffer(dst, src_size);

			src_size--;
			magic_number = ((char*) dst)[src_size];
			algo = algo_array[magic_number];
		}
	}

	if (algo->type == SCIL_COMPRESSOR_TYPE_DATATYPES){
		void * src = pick_buffer(1, total_compressors, remaining_compressors, src_adj, dest, buff_tmp1, buff_tmp2);
		void * dst = pick_buffer(0, total_compressors, remaining_compressors, src_adj, dest, buff_tmp1, buff_tmp2);

		switch(datatype){
			case(SCIL_TYPE_FLOAT):
				ret = algo->c.DNtype.decompress_float(dst, dims, src, src_size);
				break;
			case(SCIL_TYPE_DOUBLE):
				ret = algo->c.DNtype.decompress_double(dst, dims, src, src_size);
				break;
		}
		//scilU_print_buffer(dst, output_size);

		if (ret != 0) return ret;
		remaining_compressors--;
	}

	return 0;
}

void scil_determine_accuracy(enum SCIL_Datatype datatype,
	const void * restrict  data_1, const void * restrict data_2, scil_dims_t dims,
	const double relative_err_finest_abs_tolerance, scil_hints * out_hints){
	scil_hints a;
	a.absolute_tolerance = 0;
	a.relative_err_finest_abs_tolerance = 0;
	a.relative_tolerance_percent = 0;

	assert(dims.dims == 1);
	// TODO walk trough all dimensions ...

	if(datatype == SCIL_TYPE_DOUBLE){
		a.significant_bits = MANTISSA_LENGTH_DOUBLE; // in bits
		scil_determine_accuracy_1d_double((double*) data_1, (double*) data_2, dims.dims, relative_err_finest_abs_tolerance, & a);
	}else{
		a.significant_bits = MANTISSA_LENGTH_FLOAT; // in bits
		scil_determine_accuracy_1d_float((float*) data_1, (float*) data_2, scil_get_data_count(dims), relative_err_finest_abs_tolerance, & a);
	}

	// convert significant_digits in bits to 10 decimals
	a.significant_digits = scilU_convert_significant_bits_to_decimals(a.significant_bits);
	a.relative_tolerance_percent *= 100.0;

	if ( a.relative_err_finest_abs_tolerance == 0 ){
		a.relative_err_finest_abs_tolerance = a.absolute_tolerance;
	}

	*out_hints = a;
}

int scil_validate_compression(enum SCIL_Datatype datatype,
                             const void*restrict data_uncompressed,
							 scil_dims_t dims,
							 byte*restrict data_compressed,
							 const size_t compressed_size,
							 const scil_context_p ctx,
							 scil_hints * out_accuracy){

  assert(dims.length != NULL);
	// TODO, allocate uncompressed buffer...

	const uint64_t length = scil_get_data_count(dims) * DATATYPE_LENGTH(datatype) + SCIL_BLOCK_HEADER_MAX_SIZE;
	byte * data_out = (byte*)SAFE_MALLOC(4*length);
	scil_hints a;

	int ret = scil_decompress(datatype, data_out, dims, data_compressed, compressed_size, & data_out[length*2]);
	if (ret != 0){
		goto end;
	}

	if(ctx->lossless_compression_needed){
		// check bytes for identity
		ret = memcmp(data_out, (byte*) data_uncompressed, length);
		memset(&a, 0, sizeof(a));

		if (! ctx->lossless_compression_needed){
			printf("INFO: can check only for identical data as data is not a multiple of DataType\n");
		}else{
			scil_determine_accuracy(datatype, data_out, data_uncompressed, dims, ctx->hints.relative_err_finest_abs_tolerance, & a);
		}
		goto end;
	}else{
		// determine achieved accuracy
		scil_determine_accuracy(datatype, data_out, data_uncompressed, dims, ctx->hints.relative_err_finest_abs_tolerance, & a);

		const scil_hints h = ctx->hints;
		// check if tolerance level is met:
		ret = 0;
		if (a.absolute_tolerance > h.absolute_tolerance){
			ret = 1;
		}
		if (a.relative_tolerance_percent > h.relative_tolerance_percent){
			ret = 1;
		}
		if (a.relative_err_finest_abs_tolerance > h.relative_err_finest_abs_tolerance){
			ret = 1;
		}
		if (a.significant_digits < h.significant_digits){
			ret = 1;
		}
	}

end:
  	free(data_out);
	*out_accuracy = a;
	return ret;
}
