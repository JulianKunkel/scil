#include <stdio.h>
#include <math.h>
#include <time.h>

#include <scil.h>
#include <scil-util.h>

size_t count = 1000;
scil_user_hints_t hints;
scil_dims_t dims;
scil_context_t* context;

int test_quantize_float(){

    scil_context_create(&context, SCIL_TYPE_FLOAT, 0, NULL, &hints);

    size_t uncompressed_size = scil_dims_get_size(&dims, SCIL_TYPE_FLOAT);
    size_t compressed_size   = scil_get_compressed_data_size_limit(&dims, SCIL_TYPE_FLOAT);

    float* buffer_in  = (float*)malloc(uncompressed_size);
    byte* buffer_out  = (byte*)malloc(compressed_size);
    byte* buffer_tmp  = (byte*)malloc(compressed_size / 2);
    float* buffer_end = (float*)malloc(uncompressed_size);

    for(size_t i = 0; i < count; ++i){
        buffer_in[i] = ((float)rand()/RAND_MAX - 0.5f) * 200;
    }

    size_t out_size;
    scil_compress(buffer_out, compressed_size, buffer_in, &dims, &out_size, context);
    scil_decompress(SCIL_TYPE_FLOAT, buffer_end, &dims, buffer_out, out_size, buffer_tmp);

    printf("#Testing for float datatype.\n");
    printf("#Index,Value,Value after comp-decomp,Abs-difference\n");
    for (size_t i = 0; i < count; ++i) {
        float absdiff = fabsf(buffer_end[i] - buffer_in[i]);
        printf("%lu,%f,%f,%f\n", i, (double)buffer_in[i], (double)buffer_end[i], (double)absdiff);
        if ((double)absdiff > hints.absolute_tolerance) {
            printf("#FAILURE: Absolute difference bigger than tolerance.\n");
            return 1;
        }
    }

    printf("#Buffer size,Buffer size after compression\n");
    printf("%lu,%lu\n", uncompressed_size, out_size);

    free(buffer_in);
    free(buffer_out);
    free(buffer_tmp);
    free(buffer_end);

    return 0;
}
int test_quantize_double(){

    scil_context_create(&context, SCIL_TYPE_DOUBLE, 0, NULL, &hints);

    size_t uncompressed_size = scil_dims_get_size(&dims, SCIL_TYPE_DOUBLE);
    size_t compressed_size   = scil_get_compressed_data_size_limit(&dims, SCIL_TYPE_DOUBLE);

    double* buffer_in  = (double*)malloc(uncompressed_size);
    byte* buffer_out   = (byte*)malloc(compressed_size);
    byte* buffer_tmp   = (byte*)malloc(compressed_size / 2);
    double* buffer_end = (double*)malloc(uncompressed_size);

    for(size_t i = 0; i < count; ++i){
        buffer_in[i] = ((double)rand()/RAND_MAX - 0.5) * 200;
    }

    size_t out_size;
    scil_compress(buffer_out, compressed_size, buffer_in, &dims, &out_size, context);
    scil_decompress(SCIL_TYPE_DOUBLE, buffer_end, &dims, buffer_out, out_size, buffer_tmp);

    printf("#Testing for double datatype.\n");
    printf("#Index,Value,Value after comp-decomp,Abs-difference\n");
    for (size_t i = 0; i < count; ++i) {
        double absdiff = fabs(buffer_end[i] - buffer_in[i]);
        printf("%lu,%f,%f,%f\n", i, buffer_in[i], buffer_end[i], absdiff);
        if (absdiff > hints.absolute_tolerance) {
            printf("#FAILURE: Absolute difference bigger than tolerance.\n");
            return 2;
        }
    }

    printf("#Buffer size,Buffer size after compression\n");
    printf("%lu,%lu\n", uncompressed_size, out_size);

    free(buffer_in);
    free(buffer_out);
    free(buffer_tmp);
    free(buffer_end);

    return 0;
}

int main(void){

    srand((unsigned)time(NULL));

    scil_user_hints_initialize(&hints);
    hints.absolute_tolerance = 0.05;
    hints.force_compression_methods = "9";

    scil_dims_initialize_1d(&dims, count);

    unsigned char ret = 0;

    ret += test_quantize_float();
    ret += test_quantize_double();

    if (ret == 0) { printf("#No Errors\n");}
    else {
        if (ret & 1) { printf("#Error in testing float\n");}
        if (ret & 2) { printf("#Error in testing double\n");}
    }

    return ret;
}
