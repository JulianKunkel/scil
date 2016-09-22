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
int scil_create_compression_context(scil_context_p* out_ctx,
                                    enum SCIL_Datatype datatype,
                                    int special_values_count,
                                    void * special_values,
                                    const scil_user_hints_t* hints);

int scil_destroy_compression_context(scil_context_p* out_ctx);

scil_user_hints_t scil_retrieve_effective_hints(scil_context_t* ctx);

#endif // SCIL_CONTEXT_H
