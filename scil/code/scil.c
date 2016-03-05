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
#include <scil-util.h>

// known algorithms:
#include <algo/memcopy.h>
#include <algo/algo-1.h>
#include <algo/gzip.h>
#include <algo/gzalgo-1.h>

static scil_compression_algorithm * algo_array[] = {
	& algo_memcopy,
	& algo_algo1,
	& algo_gzip,
	& algo_gzalgo1,
	NULL
};

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

// This function is useful as it changes the sensitivity o the maximum double value
static void fix_double_setting(double * dbl){
	if(*dbl == SCIL_ACCURACY_DBL_IGNORE){
		*dbl = DBL_MAX;
	}
}

void scil_init_hints(scil_hints * hints){
	memset(hints, 0, sizeof(*hints));
}

int scil_create_compression_context(scil_context ** out_ctx, scil_hints * hints){
	scil_context * ctx =(scil_context*)SAFE_MALLOC(sizeof(scil_context));
	*out_ctx = ctx;

	ctx->hints.relative_tolerance_percent = hints->relative_tolerance_percent;
	ctx->hints.relative_err_finest_abs_tolerance = hints->relative_err_finest_abs_tolerance;
	ctx->hints.absolute_tolerance = hints->absolute_tolerance;
	ctx->hints.significant_digits = hints->significant_digits;
	ctx->hints.force_compression_method = hints->force_compression_method;
	ctx->lossless_compression_needed = check_compress_lossless_needed(ctx);
	fix_double_setting(& ctx->hints.relative_tolerance_percent);
	fix_double_setting(& ctx->hints.relative_err_finest_abs_tolerance);
	fix_double_setting(& ctx->hints.absolute_tolerance);
	ctx->hints.significant_digits = (ctx->hints.significant_digits == SCIL_ACCURACY_INT_IGNORE) ? 0 : ctx->hints.significant_digits;

	// verify correctness of algo_array
	int i = 0;
	for (scil_compression_algorithm ** algo = algo_array; *algo != NULL ; algo++, i++){
		if ((*algo)->magic_number != i){
			critical_error("Magic number does not match!");
		}
	}
	return 0;
}

int scil_compress(scil_context* ctx, byte* restrict dest, size_t* restrict dest_size, const DataType*restrict source, const size_t source_count){
	if (source_count == 0){
		*dest_size = 0;
		return 0;
	}

	assert(ctx != NULL);
	assert(dest != NULL);
	assert(dest_size != NULL);
	assert(source != NULL);

	const scil_hints * hints = & ctx->hints;

	// pick the best algorithm for the settings given in ctx...
	// now only one algorithm

	scil_compression_algorithm * last_algorithm;

	if (hints->force_compression_method >= 0){
			last_algorithm = algo_array[hints->force_compression_method];
	}else{
		if (ctx->lossless_compression_needed){
			// we cannot compress because data must be accurate!
			last_algorithm = & algo_memcopy;
		}else{
			last_algorithm = & algo_algo1;
		}
	}

	ctx->last_algorithm = last_algorithm;

  //Set algorithm id
  dest[0] = last_algorithm->magic_number;
  dest++;

	int ret;

	if (last_algorithm->type == SCIL_COMPRESSOR_TYPE_BASE_DATATYPE){
		ret = last_algorithm->compress(ctx, dest, dest_size, source, source_count);
	}else if (last_algorithm->type == SCIL_COMPRESSOR_TYPE_INDIVIDUAL_BYTES){
		ret = last_algorithm->compress(ctx, dest, dest_size, source, source_count * sizeof(DataType));
	}
	(*dest_size)++;

	return ret;
}

int scil_decompress(DataType*restrict dest, size_t*restrict dest_count, const byte*restrict source, const size_t source_size){
	if (source_size == 0){
		*dest_count = 0;
		return 0;
	}

	assert(dest != NULL);
	assert(dest_count != NULL);
	assert(source != NULL);

	scil_compression_algorithm * last_algorithm;

	// Read magic number (algorithm id) from header
	const uint8_t magic_number = source[0];

	// Use decompression algorithm based on algo id
	int ret;
	last_algorithm = algo_array[magic_number];

	if (last_algorithm->type == SCIL_COMPRESSOR_TYPE_BASE_DATATYPE){
		ret = last_algorithm->decompress(NULL, dest, dest_count, source + 1, source_size - 1);
	}else if (last_algorithm->type == SCIL_COMPRESSOR_TYPE_INDIVIDUAL_BYTES){
		*dest_count = (*dest_count)*sizeof(DataType);
		ret = last_algorithm->decompress(NULL, dest, dest_count, source + 1, source_size - 1);
		assert((*dest_count) % sizeof(DataType) == 0);
		*dest_count = (*dest_count) / sizeof(DataType);
	}

	return ret;
}



void scil_determine_accuracy(DataType *data_1, DataType *data_2, size_t length, scil_hints * out_hints){
	scil_hints a;
	a.absolute_tolerance = 0;
	a.significant_digits = MANTISA_LENGTH; // in bits
	a.relative_err_finest_abs_tolerance = 0;
	a.relative_tolerance_percent = 0;

	for(size_t i = 0; i < length; i++ ){
		const DataType c1 = data_1[i];
		const DataType c2 = data_2[i];
		const DataType err = c2 - c1;

		scil_hints cur;
		cur.absolute_tolerance = fabs(err);

		// determine significant digits
		{
			datatype_cast f1, f2;
			f1.f = c1;
			f2.f = c2;
			if (f1.p.sign != f2.p.sign || f1.p.exponent != f2.p.exponent){
				cur.significant_digits = 0;
			}else{
				// check mantisa, bit by bit
				//printf("%lld %lld\n", f1.p.mantisa, f2.p.mantisa);
				cur.significant_digits = MANTISA_LENGTH;
				for(int m = MANTISA_LENGTH-1 ; m >= 0; m--){
					int b1 = (f1.p.mantisa>>m) & (1);
					int b2 = (f2.p.mantisa>>m) & (1);
					// printf("%d %d\n", (int) b1, (int) b2);
					if( b1 != b2){
						cur.significant_digits = MANTISA_LENGTH - (int) m;
						break;
					}
				}
			}
		}

		// determine relative tolerance
		if (c2 == 0){
			if(c1 == 0){

			}
		}

		//double relative_err_finest_abs_tolerance;

		a.absolute_tolerance = cur.absolute_tolerance > a.absolute_tolerance ? cur.absolute_tolerance : a.absolute_tolerance;
		a.relative_err_finest_abs_tolerance = cur.relative_err_finest_abs_tolerance > a.relative_err_finest_abs_tolerance ? cur.relative_err_finest_abs_tolerance : a.relative_err_finest_abs_tolerance;
		a.relative_tolerance_percent = cur.relative_tolerance_percent > a.relative_tolerance_percent ? cur.relative_tolerance_percent : a.relative_tolerance_percent;
		a.significant_digits = cur.significant_digits < a.significant_digits ? cur.significant_digits :  a.significant_digits;
	}

	// convert significant_digits in bits to 10 decimals
	a.significant_digits = scil_convert_significant_bits_to_decimals(a.significant_digits);
	*out_hints = a;
}

int scil_validate_compression(const scil_context* ctx,
                             const size_t uncompressed_size,
                             const DataType*restrict data_uncompressed,
                             const size_t compressed_size,
                             const byte*restrict data_compressed,
													 	 scil_hints * out_accuracy )
{
	byte * data_out = (byte*)SAFE_MALLOC(uncompressed_size);
	size_t data_out_count = uncompressed_size;
	scil_hints a;

	int ret = scil_decompress((DataType*) data_out, &data_out_count, data_compressed, compressed_size);
	if (ret != 0){
		goto end;
	}

	// check length
	if (data_out_count*sizeof(DataType) != uncompressed_size){
		ret = 1;
		goto end;
	}

	if(uncompressed_size % sizeof(DataType) != 0 || ctx->lossless_compression_needed){
		// check bytes for identity
		ret = memcmp(data_out, (byte*) data_uncompressed, uncompressed_size);
		memset(&a, 0, sizeof(a));

		if (! ctx->lossless_compression_needed){
			printf("INFO: can check only for identical data as data is not a multiple of DataType\n");
		}else{
			scil_determine_accuracy((DataType*) data_out, (DataType*) data_uncompressed, uncompressed_size/sizeof(DataType), & a);
		}
		goto end;
	}else{
		// determine achieved accuracy
		scil_determine_accuracy((DataType*) data_out, (DataType*) data_uncompressed, uncompressed_size/sizeof(DataType), & a);

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
