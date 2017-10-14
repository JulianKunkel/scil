#ifndef SCIL_CHAIN_H
#define SCIL_CHAIN_H

#include <scil-cca.h>

int scilI_create_chain(scilI_chain_t* chain, const char* str_in);

int scilI_chain_is_applicable(const scilI_chain_t* chain, SCIL_Datatype_t datatype);

#endif // SCIL_CHAIN_H
