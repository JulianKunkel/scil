#include <scil-compression-algorithm.h>

#include <scil-algos.h>

scil_compression_algorithm_t* scilI_find_compressor_by_name(const char* name)
{
    int num = scil_compressor_num_by_name(name);
    if (num < 0 || num >= scil_compressors_available()) {
        return NULL;
    } else {
        return algo_array[num];
    }
}
