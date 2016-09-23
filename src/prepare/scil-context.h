#ifndef SCIL_CONTEXT_H
#define SCIL_CONTEXT_H

#include <scil-cca.h>

/**
 * \brief Creation of a compression context
 * \param datatype The datatype of the data (float, double, etc...)
 * \param out_ctx reference to the created context
 * \param hints information on the tolerable error margin
 * \pre hints != NULL
 * \param special values are special values that must be preserved, we support a list of  values
 * \return success state of the creation
 */
int scilPr_create_context(scil_context_t** out_ctx,
                          enum SCIL_Datatype datatype,
                          int special_values_count,
                          void * special_values,
                          const scil_user_hints_t* hints);

int scilPr_destroy_context(scil_context_t* out_ctx);

scil_user_hints_t scilPr_get_effective_hints(const scil_context_t* const ctx);

#endif // SCIL_CONTEXT_H
