#include <scil-context-impl.h>

#include <scil-compressor.h>
#include <scil-algo-chooser.h>
#include <scil-compression-chain.h>
#include <scil-hardware-limits.h>
#include <scil-debug.h>
#include <scil-error.h>

#include <assert.h>
#include <float.h>
#include <stdio.h>
#include <string.h>

static int initialized = 0;

static void initialize() {
  if (initialized) {
    return;
  }
  scil_initialize_compressors();

  scilU_initialize_hardware_limits();
  scilC_algo_chooser_initialize();
  initialized = 1;
}

static int check_compress_lossless_needed(scil_context_t *ctx) {
  const scil_user_hints_t hints = ctx->hints;

  if (((hints.absolute_tolerance < SCIL_ACCURACY_DBL_IGNORE || hints.absolute_tolerance > SCIL_ACCURACY_DBL_IGNORE) &&
      hints.absolute_tolerance <= SCIL_ACCURACY_DBL_FINEST) ||
      (hints.relative_err_finest_abs_tolerance <= SCIL_ACCURACY_DBL_FINEST &&
          (hints.relative_err_finest_abs_tolerance < SCIL_ACCURACY_DBL_IGNORE
              || hints.relative_err_finest_abs_tolerance > SCIL_ACCURACY_DBL_IGNORE)) ||
      (hints.relative_tolerance_percent <= SCIL_ACCURACY_DBL_FINEST &&
          (hints.relative_tolerance_percent < SCIL_ACCURACY_DBL_IGNORE
              && hints.relative_tolerance_percent > SCIL_ACCURACY_DBL_IGNORE)) ||
      (hints.significant_digits <= SCIL_ACCURACY_INT_FINEST && hints.significant_digits >= SCIL_ACCURACY_INT_FINEST) ||
      (hints.significant_bits <= SCIL_ACCURACY_INT_FINEST && hints.significant_bits >= SCIL_ACCURACY_INT_FINEST)) {
    return 1;
  }
  return 0;
}

// This function is useful as it changes the sensitivity of the maximum double
// value
static void fix_double_setting(double *dbl) {
  if (*dbl <= SCIL_ACCURACY_DBL_IGNORE && *dbl >= SCIL_ACCURACY_DBL_IGNORE) {
    *dbl = 0.0;
  }
}

int scil_context_create(scil_context_t **out_ctx,
                        SCIL_Datatype_t datatype,
                        int special_values_count,
                        special_values *sv,
                        const scil_user_hints_t *hints) {
  initialize();

  int ret = SCIL_NO_ERR;
  scil_context_t *ctx;
  *out_ctx = NULL;

  ctx = (scil_context_t *) scilU_safe_malloc(sizeof(scil_context_t));
  memset(ctx, 0, sizeof(scil_context_t));

  ctx->pipeline_params = scilU_dict_create(30);

  ctx->datatype = datatype;
  ctx->special_values_count = special_values_count;
  if (sv != NULL) {
    ctx->special_values = malloc(sizeof(special_values));
    memcpy(ctx->special_values, sv, sizeof(special_values));
  } else {
    ctx->special_values = NULL;
  }

  scil_user_hints_t *oh;
  oh = &ctx->hints;
  scil_user_hints_copy(oh, hints);

  // adjust accuracy needed
  switch (datatype) {
    case (SCIL_TYPE_FLOAT) : {
      if ((oh->significant_digits > 6) || (oh->significant_bits > 23)) {
        oh->significant_digits = SCIL_ACCURACY_INT_FINEST;
        oh->significant_bits = SCIL_ACCURACY_INT_FINEST;
      }
      break;
    }
    case (SCIL_TYPE_DOUBLE) : {
      if ((oh->significant_digits > 15) || (oh->significant_bits > 52)) {
        oh->significant_digits = SCIL_ACCURACY_INT_FINEST;
        oh->significant_bits = SCIL_ACCURACY_INT_FINEST;
      }
      break;
    }
    default: oh->significant_digits = SCIL_ACCURACY_INT_IGNORE;
      oh->significant_bits = SCIL_ACCURACY_INT_IGNORE;
  }

  // Convert between significat digits and bits
  if (oh->significant_digits != SCIL_ACCURACY_INT_IGNORE &&
      oh->significant_bits == SCIL_ACCURACY_INT_IGNORE) {

    // If bits are ignored by user, just calculate them from provided digits
    oh->significant_bits = scilU_convert_significant_decimals_to_bits(oh->significant_digits);
  } else if (oh->significant_digits == SCIL_ACCURACY_INT_IGNORE &&
      oh->significant_bits != SCIL_ACCURACY_INT_IGNORE) {

    // If digits are ignored by user, just calculate them from provided bits
    oh->significant_digits = scilU_convert_significant_bits_to_decimals(oh->significant_bits);
  } else if (oh->significant_digits != SCIL_ACCURACY_INT_IGNORE &&
      oh->significant_bits != SCIL_ACCURACY_INT_IGNORE) {

    // If the user provided both, calculate each other and take the finer ones
    int new_sig_digits = scilU_convert_significant_bits_to_decimals(oh->significant_bits);
    int new_sig_bits = scilU_convert_significant_decimals_to_bits(oh->significant_digits);

    oh->significant_digits = max(new_sig_digits, oh->significant_digits);
    oh->significant_bits = max(new_sig_bits, oh->significant_bits);
  }

  ctx->lossless_compression_needed = check_compress_lossless_needed(ctx);
  //fix_double_setting(&oh->relative_tolerance_percent);
  //fix_double_setting(&oh->relative_err_finest_abs_tolerance);
  //fix_double_setting(&oh->absolute_tolerance);
  // TODO handle float differently.
  // Why? hints can be double while compressing float-data.

  if (oh->force_compression_methods != NULL) {
    // now we can prefill the compression pipeline
    ret = scilU_chain_create(&ctx->chain, hints->force_compression_methods);
    if (ret == SCIL_NO_ERR) {
      ret = scilU_chain_is_applicable(&ctx->chain, datatype);
      if (ret == SCIL_NO_ERR) {
        oh->force_compression_methods = strdup(oh->force_compression_methods);
      }
    }
  }

  if (ret == SCIL_NO_ERR) {
    *out_ctx = ctx;
  } else {
    free(ctx);
  }

  return ret;
}

int scil_destroy_context(scil_context_t *out_ctx) {
  free(out_ctx->hints.force_compression_methods);
  free(out_ctx);
  out_ctx = NULL;

  return SCIL_NO_ERR;
}

scil_user_hints_t scil_get_effective_hints(const scil_context_t *ctx) {
  return ctx->hints;
}
