#include <stdio.h>

#include <sz.h>
#include <zlib.h>

int main(void){
  double data[] = {1,2,3,4,5};
  int out_size;

  struct sz_params p;
  memset(& p, -1, sizeof(p));
  p.dataEndianType = LITTLE_ENDIAN_DATA;
  p.max_quant_intervals = 65536;
  p.quantization_intervals = 0;
  //p.sol_ID = 101;
  p.layers = 1;
  p.sampleDistance = 100;
  p.predThreshold = 0.97;
  p.offset = 0;
  p.szMode = SZ_BEST_COMPRESSION;
  //p.gzipMode = Z_BEST_SPEED;
  p.errorBoundMode = ABS; //ABS_AND_REL
  p.absErrBound = 0.0001;
  p.relBoundRatio = 0.001;
  p.pw_relBoundRatio = 0.000001;
  p.segment_size = 32;

  unsigned char out_buff[102400];

  SZ_Init_Params(& p);
  //SZ_Init("sz.config");
  int ret = SZ_compress_args2(SZ_DOUBLE, data, out_buff, & out_size, ABS, 1.0, 0.0,  0,0,0,0, 5);
  printf("%d %d\n", ret, out_size);

  double * result = malloc(sizeof(double)* 10);

  ret = SZ_decompress_args(SZ_DOUBLE, out_buff, out_size, (void*) result, 0,0,0,0,5);
  for(int i=0; i < 5; i++){
    printf("%f\n", result[i]);
  }

  SZ_Finalize();
  return 0;
}
