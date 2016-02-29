#include <string.h>

#include <scil-internal.h>
#include <util.h>

// known algorithms:
#include <algo/memcopy.h>
#include <algo/algo-1.h>
#include <algo/gzip.h>
#include <algo/gzalgo-1.h>

int scil_create_compression_context(scil_context ** out_ctx, scil_hints * hints){
	scil_context * ctx =(scil_context*)SAFE_MALLOC(sizeof(scil_context));
	*out_ctx = ctx;

	ctx->hints.relative_tolerance_percent = hints->relative_tolerance_percent;
	ctx->hints.relative_err_finest_abs_tolerance = hints->relative_err_finest_abs_tolerance;
	ctx->hints.absolute_tolerance = hints->absolute_tolerance;
	ctx->hints.significant_digits = hints->significant_digits;
	ctx->hints.force_compression_method = hints->force_compression_method;

	return 0;
}

int scil_compress(scil_context* ctx, byte* restrict dest, size_t* restrict dest_size, const double*restrict source, const size_t source_count){

	assert(ctx != NULL);
	assert(dest != NULL);
	assert(dest_size != NULL);
	assert(source != NULL);

	const scil_hints * hints = & ctx->hints;

	// pick the best algorithm for the settings given in ctx...
	// now only one algorithm

	scil_compression_algorithm * last_algorithm;

	if (hints->force_compression_method >= 0){
		switch (hints->force_compression_method) {
			case 0: last_algorithm = & algo_memcopy; break;
			case 1: last_algorithm = & algo_algo1; break;
			case 2: last_algorithm = & algo_gzip; break;
			case 3: last_algorithm = & algo_gzalgo1; break;
			default: last_algorithm = & algo_memcopy;
		}
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

	int ret = last_algorithm->compress(ctx, dest, dest_size, source, source_count);
	dest_size++;

	return ret;
}

int scil_decompress(double*restrict dest, size_t*restrict dest_count, const byte*restrict source, const size_t source_size){

	assert(dest != NULL);
	assert(dest_count != NULL);
	assert(source != NULL);

	scil_compression_algorithm * last_algorithm;

	// Read magic number (algorithm id) from header
	uint8_t magic_number = source[0];

	// Use decompression algorithm based on algo id
	switch(magic_number){

		case 0: last_algorithm = & algo_memcopy; break;
		case 1: last_algorithm = & algo_algo1; break;
		case 2: last_algorithm = & algo_gzip; break;
		case 3: last_algorithm = & algo_gzalgo1; break;
	}

	return last_algorithm->decompress(NULL, dest, dest_count, source + 1, source_size - 1);
}

int scil_validate_compression(const scil_context* ctx,
                             const size_t uncompressed_size,
                             const double*restrict data_uncompressed,
                             const size_t compressed_size,
                             const double*restrict data_compressed )
{
	return 1;
}
