#include <scil-swager.h>

#include <scil-util.h>

#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <assert.h>

int main(void){

    const uint32_t count = 100;
    const uint8_t max_bits_per_value = 63;

    const uint64_t buf_size = count * sizeof(uint64_t);

    uint64_t* buf_in  = (uint64_t*)SAFE_MALLOC(buf_size);
    byte*     buf_out = (byte*)SAFE_MALLOC(buf_size);
    uint64_t* buf_end = (uint64_t*)SAFE_MALLOC(buf_size);

    srand((unsigned)time(NULL));

    for(uint8_t i = 1; i < max_bits_per_value; ++i)
    {
        for(uint64_t i = 0; i < count; ++i)
        {
            buf_in[i] = (uint64_t)floor( ((double)rand() / RAND_MAX) * (1 << i) );
        }

        scil_swage(buf_out, buf_in, count, i);
        scil_unswage(buf_end, buf_out, count, i);

        for(uint64_t i = 0; i < count; ++i)
        {
            assert(buf_in[i] == buf_end[i]);
        }
    }

    free(buf_in);
    free(buf_out);
    free(buf_end);

    return 0;
}
