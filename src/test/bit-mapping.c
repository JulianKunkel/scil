#include <stdint.h>

#include <scil-util.c>

int main(void){

    compute_significant_bit_mapping();

    printf("#Significant decimals,required bits\n");
    for(uint8_t i = 0; i < MANTISSA_MAX_LENGTH_P1; ++i){
        printf("%d,%d\n", i, sig_bits[i]);
    }

    printf("#Significant bits,required decimals\n");
    for(uint8_t i = 0; i < MANTISSA_MAX_LENGTH_P1; ++i){
        printf("%d,%d\n", i, sig_decimals[i]);
    }

    return 0;
}
