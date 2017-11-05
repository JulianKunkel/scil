#include <scil-swager.h>

#include <scil-util.h>

#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <assert.h>

int main(void){

    const uint32_t count = 1000;
    const uint8_t max_bits_per_value = 64;

    const uint64_t buf_size = count * sizeof(uint64_t);

    uint64_t* buf_in  = (uint64_t*)scilU_safe_malloc(buf_size);
    byte*     buf_out = (byte*)scilU_safe_malloc(buf_size);
    uint64_t* buf_end = (uint64_t*)scilU_safe_malloc(buf_size);

    srand((unsigned)time(NULL));

    for(uint8_t i = 1; i <= max_bits_per_value; ++i)
    {
        uint64_t l = 1UL << (i - 1);
        printf("Test input values for %u bits:\n", i);
        for(uint64_t j = 0; j < count; ++j)
        {
            buf_in[j] = (uint64_t)round( ((double)rand() / RAND_MAX) * l );
            printf("%04lu: 0x%016lX\n", j, buf_in[j]);
        }
        printf("\n");

        scil_swage(buf_out, buf_in, count, i);
        scil_unswage(buf_end, buf_out, count, i);

        printf("Values after swage-unswage-cycle for %u bits:\n", i);
        for(uint64_t j = 0; j < count; ++j)
        {
            printf("%04lu: 0x%016lX\n", j, buf_end[j]);
        }
        printf("\n");

        for(uint64_t j = 0; j < count; ++j)
        {
            if(buf_in[j] != buf_end[j])
            {
                printf("Assertion failed at index %lu:\n", j);
                printf("Expected value: 0x%016lX\n", buf_in[j]);
                printf("Actual value:   0x%016lX\n", buf_end[j]);
                return 1;
            }
        }
    }

    printf("Success!\n");

    free(buf_in);
    free(buf_out);
    free(buf_end);

    return 0;
}
