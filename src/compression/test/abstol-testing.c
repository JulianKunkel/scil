#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

double rounder(double val, double min, double abstol){
  uint64_t enc = (((uint64_t) ( ((double) (val - min)) / abstol )) + 1)>>1;
  double result = (enc * abstol*2.0) + min;
  printf("%.2f -> %.2f -> %lld \n", val, result, enc);
}

int main(){
  rounder(5.1, 0.0, 1.0);
  rounder(4.9, 0.0, 1.0);
  rounder(2.1, 0.0, 1.0);
  rounder(1.01, 0.0, 1.0);
  rounder(0.99, 0.0, 1.0);
  rounder(0.51, 0.0, 1.0);
  rounder(0.49, 0.0, 1.0);
  rounder(0.0, 0.0, 1.0);
  return 0;
}
