#include "scil.h"

int scil_create_compression_context(scil_context * out_ctx, scil_hints * hints){

	out_ctx->hints = hints;

	return 0;
}

int scil_compress(scil_context * ctx, size_t count, float/double * data, char * compressed_buf, size_t * out_size){

	return 0;
}

int scil_decompress(scil_context * ctx, char * compressed_buf, float/double * data_out, size_t * out_size){

	return 0;
}