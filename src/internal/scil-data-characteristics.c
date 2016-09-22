#include <scil-data-characteristics.h>

#include <scil-internal.h>

#include <algo/lz4fast.h>

float scilI_determine_randomness(void* source, size_t in_size, byte *restrict buffer, size_t buffer_size)
{
    // We may want to use https://en.wikipedia.org/wiki/Randomness_tests
    int ret = scil_lz4fast_compress(NULL, buffer, &buffer_size, source, in_size);
    if (ret == 0){
        double rnd = buffer_size * 100.0 / in_size;
        return rnd;
    }else{
        critical("lz4fast error to determine randomness: %d\n", ret);
    }
}
