#ifndef SCIL_CONTEXT_H
#define SCIL_CONTEXT_H

#include <scil-datatypes.h>
#include <scil-user-hints.h>
#include <scil-dims.h>
#include <scil-util.h>

struct scil_context;
typedef struct scil_context scil_context_t;

/**
 * \brief Creation of a compression context
 * \param datatype The datatype of the data (float, double, etc...)
 * \param out_ctx reference to the created context
 * \param hints information on the tolerable error margin
 * \pre hints != NULL
 * \param special values are special values that must be preserved, we support a list of  values
 * \return success state of the creation
 */
int scil_context_create(scil_context_t **out_ctx,
                        SCIL_Datatype_t datatype,
                        int special_values_count,
                        special_values *special_values,
                        const scil_user_hints_t *hints);

int scil_destroy_context(scil_context_t *out_ctx);

scil_user_hints_t scil_get_effective_hints(const scil_context_t *ctx);

#endif // SCIL_CONTEXT_H
