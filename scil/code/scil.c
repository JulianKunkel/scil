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

#include <scil-internal.h>

// known algorithms:
#include <algo/algo-memcopy.h>
#include <algo/algo-1.h>
#include <algo/algo-gzip.h>
#include <algo/algo-2.h>
#include <algo/algo-fpzip.h>
#include <algo/algo-zfp.h>

// this file is automatically created
#include "scil-dtypes.h"

scil_compression_algorithm * algo_array[] = {
	& algo_memcopy,
	& algo_algo1,
	& algo_gzip,
	& algo_algo2,
	& algo_fpzip,
	& algo_zfp,
	NULL
};

#pragma GCC diagnostic ignored "-Wfloat-equal"
static int check_compress_lossless_needed(scil_context * ctx){
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

static uint8_t relative_tolerance_to_significant_bits(double rel_tol){

	return (uint8_t)ceil(log2(100.0 / rel_tol));
}

static double significant_bits_to_relative_tolerance(uint8_t sig_bits){

	return 100.0 / exp2(sig_bits);
}

static scil_compression_algorithm* pick_best_algorithm(){
	// TODO: implement this
	return NULL;
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
}

void scil_hints_print(scil_hints * h){
	printf("Hints: \n\trelative_tolerance_percent:%f \n\trelative_err_finest_abs_tolerance:%f \n\tabsolute_tolerance:%f \n\tsignificant_digits (after 1. so in the mantisa):%d \n\tsignificant_bits (in the mantisa):%d\n",
		h->relative_tolerance_percent, h->relative_err_finest_abs_tolerance, h->absolute_tolerance, h->significant_digits, h->significant_bits);
}

int scil_create_compression_context(scil_context ** out_ctx, scil_hints * hints){
	scil_context * ctx =(scil_context*)SAFE_MALLOC(sizeof(scil_context));
	*out_ctx = ctx;
	scil_hints * ohints = & ctx->hints;

	memcpy(ohints, hints, sizeof(scil_hints));

	// convert between significant digits and bits
	if (ohints->significant_digits != SCIL_ACCURACY_INT_IGNORE){
		ohints->significant_bits = max(ohints->significant_bits, scil_convert_significant_decimals_to_bits(ohints->significant_digits) );
	}

	if(ohints->relative_tolerance_percent != SCIL_ACCURACY_DBL_IGNORE){
		ohints->significant_bits = max(ohints->significant_bits, relative_tolerance_to_significant_bits(ohints->relative_tolerance_percent));
	}

	if (ohints->significant_bits != SCIL_ACCURACY_INT_IGNORE){
	 	if(ohints->significant_digits == SCIL_ACCURACY_INT_IGNORE){
			ohints->significant_digits = scil_convert_significant_bits_to_decimals(ohints->significant_bits);

			// we need to round the bits properly to decimals, i.e., 1 bit precision in the mantisa requires 1 decimal digit.
			const int newbits = scil_convert_significant_decimals_to_bits(ohints->significant_digits);
			if ( newbits < ohints->significant_bits ){
				ohints->significant_digits = scil_convert_significant_bits_to_decimals(ohints->significant_bits) + 1;
			}
		}
		if(ohints->relative_tolerance_percent == SCIL_ACCURACY_DBL_IGNORE){
			ohints->relative_tolerance_percent = significant_bits_to_relative_tolerance(ohints->significant_bits);
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
			critical_error("Magic number does not match!");
		}
	}
	}

	return 0;
}

int scil_compress(enum SCIL_Datatype datatype, byte* restrict dest, size_t* restrict dest_size,
  void*restrict source, scil_dims_t dims, scil_context* ctx){

	if (dims.dims == 0){
		*dest_size = 0;
		return 0;
	}

	assert(dims.length != NULL);

	assert(ctx != NULL);
	assert(dest != NULL);
	assert(dest_size != NULL);
	assert(source != NULL);

	const scil_hints * hints = & ctx->hints;

	scil_compression_algorithm * last_algorithm;

	if (hints->force_compression_method >= 0){
			last_algorithm = algo_array[hints->force_compression_method];
	}else{
		if (ctx->lossless_compression_needed){
			// we cannot compress because data must be accurate!
			last_algorithm = & algo_memcopy;
		}else{
			// TODO: pick the best algorithm for the settings given in ctx...
			last_algorithm = pick_best_algorithm();
		}
	}

	ctx->last_algorithm = last_algorithm;

	//Set algorithm id
	dest[0] = last_algorithm->magic_number;
	dest++;

	int ret;

	if (last_algorithm->type == SCIL_COMPRESSOR_TYPE_DATATYPES){
		switch(datatype){
			case(SCIL_FLOAT):
				ret = last_algorithm->c.DNtype.compress_float(ctx, dest, dest_size, source, dims);
				break;
			case(SCIL_DOUBLE):
				ret = last_algorithm->c.DNtype.compress_double(ctx, dest, dest_size, source, dims);
				break;
		}
	}else if (last_algorithm->type == SCIL_COMPRESSOR_TYPE_INDIVIDUAL_BYTES){
		ret = last_algorithm->c.Btype.compress(ctx, dest, dest_size, (byte *) source, scil_get_data_count(dims) * datatype_length(datatype));
	}
	(*dest_size)++;

	return ret;
}

int scil_decompress(enum SCIL_Datatype datatype, void*restrict dest, scil_dims_t dims,
    byte*restrict source, const size_t source_size){

	//assert(dims.dims == 1);

	if (dims.dims == 0){
		return 0;
	}

	assert(dest != NULL);
	assert(source != NULL);

	scil_compression_algorithm * last_algorithm;

	// Read magic number (algorithm id) from header
	const uint8_t magic_number = source[0];

	// Use decompression algorithm based on algo id
	int ret;
	last_algorithm = algo_array[magic_number];

	if (last_algorithm->type == SCIL_COMPRESSOR_TYPE_DATATYPES){
		switch(datatype){
		case(SCIL_FLOAT):
			ret = last_algorithm->c.DNtype.decompress_float(NULL, dest, dims, source + 1, source_size - 1);
			break;
		case(SCIL_DOUBLE):
			ret = last_algorithm->c.DNtype.decompress_double(NULL, dest, dims, source + 1, source_size - 1);
			break;
		}

	}else if (last_algorithm->type == SCIL_COMPRESSOR_TYPE_INDIVIDUAL_BYTES){
		ret = last_algorithm->c.Btype.decompress(NULL, (byte *) dest, scil_get_data_count(dims) * datatype_length(datatype), source + 1, source_size - 1);
	}

	return ret;
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

	if(datatype == SCIL_DOUBLE){
		a.significant_bits = MANTISA_LENGTH_double; // in bits
		scil_determine_accuracy_1d_double((double*) data_1, (double*) data_2, dims.dims, relative_err_finest_abs_tolerance, & a);
	}else{
		a.significant_bits = MANTISA_LENGTH_float; // in bits
		scil_determine_accuracy_1d_float((float*) data_1, (float*) data_2, scil_get_data_count(dims), relative_err_finest_abs_tolerance, & a);
	}

	// convert significant_digits in bits to 10 decimals
	a.significant_digits = scil_convert_significant_bits_to_decimals(a.significant_bits);
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
							 const scil_context* ctx,
							 scil_hints * out_accuracy){

  	assert(dims.length != NULL); // TODO, allocate uncompressed buffer...

	const uint64_t length = scil_get_data_count(dims) * datatype_length(datatype) + SCIL_BLOCK_HEADER_MAX_SIZE;
	byte * data_out = (byte*)SAFE_MALLOC(length);
	scil_hints a;

	int ret = scil_decompress(datatype, data_out, dims, data_compressed, compressed_size);
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


static char sig_bits[MANTISSA_MAX_LENGTH] = {0};
static char sig_decimals[MANTISSA_MAX_LENGTH] = {0};

static void compute_significant_bit_mapping(){
	if(sig_bits[0] == 0){
		int c = 0;
		for(int i=0; i < MANTISSA_MAX_LENGTH; i++){
			int v;
			if (i%3 == 0){
				v = 4;
			}else{
				v = 3;
			}
			for(int b=c; b < c+v; b++){
				sig_decimals[b] = (char)i;
			}
			sig_bits[i] = (char) c;
			c += v;
		}
	}
}

int scil_convert_significant_decimals_to_bits(int decimals){
	compute_significant_bit_mapping();
	return sig_bits[decimals];
}

int scil_convert_significant_bits_to_decimals(int bits){
  assert(bits > 0);
	// compute mapping between decimals and bits
	compute_significant_bit_mapping();
	return sig_decimals[bits];
}
