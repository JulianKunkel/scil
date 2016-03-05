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

int scil_create_compression_context(scil_context ** out_ctx, scil_hints * hints){
	scil_context * ctx =(scil_context*)SAFE_MALLOC(sizeof(scil_context));
	*out_ctx = ctx;

	ctx->hints.relative_tolerance_percent = hints->relative_tolerance_percent;
	ctx->hints.relative_err_finest_abs_tolerance = hints->relative_err_finest_abs_tolerance;
	ctx->hints.absolute_tolerance = hints->absolute_tolerance;
	ctx->hints.significant_digits = hints->significant_digits;
	ctx->hints.force_compression_method = hints->force_compression_method;

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
		if (hints->absolute_tolerance == 0.0 || hints->relative_err_finest_abs_tolerance == 0.0 || hints->relative_tolerance_percent == 0.0 || hints->significant_digits > 20){
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
		dest_count = (*dest_count) / sizeof(DataType);
	}

	return ret;
}

int scil_validate_compression(const scil_context* ctx,
                             const size_t uncompressed_size,
                             const DataType*restrict data_uncompressed,
                             const size_t compressed_size,
                             const DataType*restrict data_compressed )
{
	return 1;
}
