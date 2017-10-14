#ifndef SCIL_ALGOS_H
#define SCIL_ALGOS_H

#include <scil-cca.h>

scilI_algorithm_t* scil_get_compressor(int number);
scilI_algorithm_t* scilI_find_compressor_by_name(const char* name);

void scil_initialize_compressors();

/*
 \brief Returns the number of available compression schemes.
 */
int scilU_get_available_compressor_count();

const char* scilU_get_compressor_name(int number);
int scilU_get_compressor_number(const char* name);

#endif // SCIL_ALGOS_H
