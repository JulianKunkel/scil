#ifndef SCIL_SWAGE_H_
#define SCIL_SWAGE_H_

#include <scil-algorithm-impl.h>

// Repeat for each data type

int scil_swage_compress_<DATATYPE>(const scil_context_t* ctx,
                                   byte * restrict dest,
                                   size_t* restrict out_size,
                                   <DATATYPE>*restrict source,
                                   const scil_dims_t* dims);

int scil_swage_decompress_<DATATYPE>(<DATATYPE>*restrict dest,
                                     scil_dims_t* dims,
                                     byte*restrict source,
                                     const size_t in_size);

// End repeat

extern scilU_algorithm_t algo_swage;

#endif /* SCIL_SWAGE_H_<DATATYPE> */
