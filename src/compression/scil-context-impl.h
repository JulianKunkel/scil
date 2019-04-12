#ifndef SCIL_CONTEXT_IMPL_H
#define SCIL_CONTEXT_IMPL_H

#include <scil-context.h>
#include <scil-compression-chain.h>

struct scil_context {
  int lossless_compression_needed;
  enum SCIL_Datatype datatype;
  scil_user_hints_t hints;

  /** \brief Special values are special values that must be preserved, we support a list of  values */
  int special_values_count;
  special_values *special_values;

  /** \brief The last compressor used, could be used for debugging */
  scil_compression_chain_t chain;

  /** \brief Dictionary for pipeline internal parameters */
  scilU_dict_t *pipeline_params;
};

#endif // SCIL_CONTEXT_H
