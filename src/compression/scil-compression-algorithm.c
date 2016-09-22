#include <scil-compression-algorithm.h>

#include <scil-compressors.h>

scil_compression_algorithm_t* scilI_find_compressor_by_name(const char* name)
{
    int num = scilU_get_compressor_number(name);
    if (num < 0 || num >= scilU_get_available_compressor_count()) {
        return NULL;
    } else {
        return algo_array[num];
    }
}
