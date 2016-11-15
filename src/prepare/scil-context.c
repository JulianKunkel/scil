#include <scil-context.h>

#include <scil-compressors.h>
#include <scil-algo-chooser.h>
#include <scil-chain.h>
#include <scil-hardware-limits.h>
#include <scil-util.h>

#include <assert.h>
#include <float.h>
#include <stdio.h>
#include <string.h>

static int initialized = 0;

static void initialize()
{
    if (initialized) {
        return;
    }

    // verify correctness of algo_array
    int i = 0;
    for (scilI_algorithm_t **algo = algo_array; *algo != NULL;
         algo++, i++) {
        if ((*algo)->compressor_id != i) {
					printf("id_%i i=%i",(*algo)->compressor_id,i);
            scilU_critical_error("compressor ID does not match!");
        }
        if ((*algo)->type == SCIL_COMPRESSOR_TYPE_INDIVIDUAL_BYTES) {
            // we expect that all byte compressors are lossless
            (*algo)->is_lossy = 0;
        }
    }

    scilI_initialize_hardware_limits();
    scilC_algo_chooser_initialize();
    initialized = 1;
}

static int check_compress_lossless_needed(scil_context_t* ctx)
{
    const scil_user_hints_t hints = ctx->hints;

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

int scilPr_create_context(scil_context_t** out_ctx,
                          enum SCIL_Datatype datatype,
                          int special_values_count,
                          void* special_values,
                          const scil_user_hints_t* hints)
{
    initialize();

    int ret = SCIL_NO_ERR;
    scil_context_t* ctx;
    *out_ctx = NULL;

    ctx = (scil_context_t*)SAFE_MALLOC(sizeof(scil_context_t));
    memset(&ctx->chain, 0, sizeof(ctx->chain));
    ctx->datatype = datatype;
		ctx->special_values_count = special_values_count;
		if (ctx->special_values_count > 0){
			assert(special_values != NULL);
			ctx->special_values = special_values;
		}else{
			ctx->special_values = NULL;
		}

    scil_user_hints_t* oh;
    oh = & ctx->hints;
	scilPr_copy_user_hints(oh, hints);

	// adjust accuracy needed
	switch(datatype){
	case (SCIL_TYPE_FLOAT) : {
        if ((oh->significant_digits > 6) || (oh->significant_bits > 23)) {
            oh->significant_digits = SCIL_ACCURACY_INT_FINEST;
            oh->significant_bits   = SCIL_ACCURACY_INT_FINEST;
        }
		break;
	}
	case (SCIL_TYPE_DOUBLE) : {
        if ((oh->significant_digits > 15) || (oh->significant_bits > 52)) {
            oh->significant_digits = SCIL_ACCURACY_INT_FINEST;
            oh->significant_bits   = SCIL_ACCURACY_INT_FINEST;
        }
		break;
	}
	default:
    	oh->significant_digits = SCIL_ACCURACY_INT_IGNORE;
    	oh->significant_bits   = SCIL_ACCURACY_INT_IGNORE;
	}

	// Convert between significat digits and bits
	if(	oh->significant_digits != SCIL_ACCURACY_INT_IGNORE &&
	   	oh->significant_bits   == SCIL_ACCURACY_INT_IGNORE ){

		// If bits are ignored by user, just calculate them from provided digits
	    oh->significant_bits = scilU_convert_significant_decimals_to_bits(oh->significant_digits);
	}
	else if( oh->significant_digits == SCIL_ACCURACY_INT_IGNORE &&
	   		 oh->significant_bits   != SCIL_ACCURACY_INT_IGNORE ){

		// If digits are ignored by user, just calculate them from provided bits
		oh->significant_digits = scilU_convert_significant_bits_to_decimals(oh->significant_bits);
	}
	else if( oh->significant_digits != SCIL_ACCURACY_INT_IGNORE &&
	   		 oh->significant_bits   != SCIL_ACCURACY_INT_IGNORE ){

		// If the user provided both, calculate each other and take the finer ones
	    int new_sig_digits = scilU_convert_significant_bits_to_decimals(oh->significant_bits);
	    int new_sig_bits   = scilU_convert_significant_decimals_to_bits(oh->significant_digits);

		oh->significant_digits = max(new_sig_digits, oh->significant_digits);
		oh->significant_bits   = max(new_sig_bits,   oh->significant_bits);
	}

    ctx->lossless_compression_needed = check_compress_lossless_needed(ctx);
    fix_double_setting(&oh->relative_tolerance_percent);
    fix_double_setting(&oh->relative_err_finest_abs_tolerance);
    fix_double_setting(&oh->absolute_tolerance);
    // TODO handle float differently.
	// Why? hints can be double while compressing float-data.

    if (oh->force_compression_methods != NULL) {
        // now we can prefill the compression pipeline
        ret = scilI_create_chain(&ctx->chain, hints->force_compression_methods);

        oh->force_compression_methods = strdup(oh->force_compression_methods);
    }

    if (ret == SCIL_NO_ERR) {
        *out_ctx = ctx;
    } else {
        free(ctx);
    }

    return ret;
}

int scilPr_destroy_context(scil_context_t* out_ctx)
{
    free(out_ctx->hints.force_compression_methods);
    free(out_ctx);
    out_ctx = NULL;

    return SCIL_NO_ERR;
}

scil_user_hints_t scilPr_get_effective_hints(const scil_context_t* ctx)
{
    return ctx->hints;
}
