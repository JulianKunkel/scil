#ifndef SCIL_QUANTIZE_H_
#define SCIL_QUANTIZE_H_

#include <scil-algorithm-impl.h>

// Repeat for each data type

int scil_quantize_compress_<DATATYPE>(const scil_context_t* ctx,
                                      int64_t * restrict dest,
                                      size_t* restrict out_size,
                                      <DATATYPE>*restrict source,
                                      const scil_dims_t* dims);

int scil_quantize_decompress_<DATATYPE>(<DATATYPE>*restrict dest,
                                        scil_dims_t* dims,
                                        int64_t*restrict source,
                                        const size_t in_size);

// End repeat

extern scilU_algorithm_t algo_quantize;

#endif /* SCIL_QUANTIZE_H_<DATATYPE> */
