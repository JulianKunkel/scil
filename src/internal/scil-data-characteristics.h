#ifndef SCIL_DATA_CHARACTERISTICS_H
#define SCIL_DATA_CHARACTERISTICS_H

#include <scil-datatypes.h>

#include <stdlib.h>

float scilI_determine_randomness(void* source, size_t in_size, byte *restrict buffer, size_t buffer_size);

#endif // SCIL_DATA_CHARACTERISTICS_H
