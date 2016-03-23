#include <scil.h>

#include <stdint.h>

#include <time.h>
#include <math.h>

#pragma GCC diagnostic ignored "-Wunused-parameter"
int main(int argc, char** argv){

    double rel_tol = 0.001;
    uint8_t sig_bits = (uint8_t)ceil(log2(100.0 / rel_tol));

    printf("%d\n", sig_bits);

    return 0;
}
