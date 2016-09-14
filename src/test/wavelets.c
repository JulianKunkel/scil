#include <stdio.h>

#include <scil.h>
#include <scil-util.h>
#include <string.h>
#include <math.h>


int main(void){

    size_t count1 = 8;
    size_t count2 = 8;
    float mse=0;
    size_t i,j;

    scil_hints hints;
    scil_init_hints(&hints);
    hints.absolute_tolerance = 0.005;
    hints.force_compression_methods = "11";

    scil_context_p context;
    scil_create_compression_context(&context, SCIL_TYPE_FLOAT, 0, NULL, &hints);

    scil_dims dims;
    scil_init_dims_2d(&dims, count1, count2);

    printf("TEST WAVELETS\n>COMPRESSION\n");

    size_t uncompressed_size = scil_get_data_size(SCIL_TYPE_FLOAT, &dims);
    size_t compressed_size   = scil_compress_buffer_size_bound(SCIL_TYPE_FLOAT, &dims);

    float* buffer_in  = (float*)malloc(uncompressed_size);
    byte* buffer_out   = (byte*)malloc(compressed_size);
    byte* buffer_tmp   = (byte*)malloc(compressed_size / 2);
    float* buffer_end = (float*)malloc(uncompressed_size);

    for(i = 0; i < count1; ++i){
      for(j = 0; j < count2; ++j){
        *(buffer_in+i*count2+j) = ((float)rand()/RAND_MAX - 0.5) * 200;
      }
    }

    printf("Input\n");
    for(int i=0;i<count1;i++) {
      for(int j=0;j<count2;j++) {
        printf("%f ",*(float*)(buffer_in+i*count2+j));
      }
    }

    size_t out_size;
    scil_compress(buffer_out, compressed_size, buffer_in, &dims, &out_size, context);

    printf("\n>DECOMPRESSION\n");

    scil_decompress(SCIL_TYPE_FLOAT, buffer_end, &dims, buffer_out, out_size, buffer_tmp);

    printf("Output\n");
    for(i=0;i<count1;i++) {
      for(j=0;j<count2;j++) {
        printf("%f ",*(float*)(buffer_end+i*count2+j));
      }
    }


    mse=0;
    for(i=0;i<count1;i++)
      for(j=0;j<count2;j++) {
        mse+=((*(buffer_in+i*count2+j)) - (*(buffer_end+i*count2+j))) * ((*(buffer_in+i*count2+j)) - (*(buffer_end+i*count2+j)));
      }
    mse/=count1*count2;

    printf("\nThe mse due to transformation and inverse transformation is: \n");
    printf("\t%8.4f, ( %8.4f dB)\n ",mse,10*log10(255*255/mse));
    printf("(Very small amount of mse is OK due to numerical errors.)\n");

    /*
    printf("#Buffer size,Buffer size after compression\n");
    printf("%lu,%lu\n", uncompressed_size, out_size);*/

    free(buffer_in);
    free(buffer_out);
    free(buffer_tmp);
    free(buffer_end);

    //scil_destroy_compression_context(&context);

    return 0;
}
