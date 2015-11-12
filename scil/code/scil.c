#include "scil-internal.h"
#include "util.h"


int scil_create_compression_context(scil_context ** out_ctx, scil_hints * hints){
	scil_context * ctx =(scil_context*)SAFE_MALLOC(sizeof(scil_context));
	*out_ctx = ctx;

	ctx->hints.relative_tolerance_percent = hints->relative_tolerance_percent;
	ctx->hints.absolute_tolerance = hints->absolute_tolerance;

	return 0;
}

int scil_compress(const scil_context* ctx, char** compressed_buf_out, size_t* out_size, const double* data_in, const size_t in_size){
	assert(ctx != NULL);
	assert(data_in != NULL);

	int ret;

	// pick the best algorithm for the settings given in ctx...
	// now only one algorithm

	ret = scil_algo1_compress(ctx, compressed_buf_out, out_size, data_in, in_size);

	return ret;
}

int scil_decompress(const scil_context* ctx, double* data_out, const size_t* out_size, const char* compressed_buf_in, const size_t in_size){
	assert(ctx != NULL);
	assert(data_out != NULL);

	int ret;

	ret = scil_algo1_decompress(ctx, data_out, out_size, compressed_buf_in, in_size);
	return ret;
}

int scil_validate_compression(const scil_context* ctx,
                             const size_t uncompressed_size,
                             const double*restrict data_uncompressed,
                             const size_t compressed_size,
                             const double*restrict data_compressed )
{
	return 1;
}
