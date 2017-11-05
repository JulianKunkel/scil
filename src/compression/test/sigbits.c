#include <stdio.h>
#include <time.h>

#include <scil.h>
#include <scil-util.h>

int main(void){

    size_t count = 1000;

    scil_user_hints_t hints;
    scil_user_hints_initialize(&hints);
    //hints.relative_tolerance_percent = 5.0;
    hints.significant_bits = 11;
    hints.force_compression_methods = "3";

    scil_context_t* context;
    scil_context_create(&context, SCIL_TYPE_DOUBLE, 0, NULL, &hints);

    scil_dims_t dims;
    scil_dims_initialize_1d(&dims, count);

    size_t uncompressed_size = scil_dims_get_size(&dims, SCIL_TYPE_DOUBLE);
    size_t compressed_size   = scil_get_compressed_data_size_limit(&dims, SCIL_TYPE_DOUBLE);

    double* buffer_in  = (double*)malloc(uncompressed_size);
    byte* buffer_out   = (byte*)malloc(compressed_size);
    byte* buffer_tmp   = (byte*)malloc(compressed_size / 2);
    double* buffer_end = (double*)malloc(uncompressed_size);

    srand((unsigned)time(NULL));
    for(size_t i = 0; i < count; ++i){
        //buffer_in[i] = ((double)rand()/RAND_MAX - 0.5) * 200;
        buffer_in[i] = -100.0 + (double)rand()/RAND_MAX * 200.0;
    }

    size_t out_size;
    scil_compress(buffer_out, compressed_size, buffer_in, &dims, &out_size, context);
    scil_decompress(SCIL_TYPE_DOUBLE, buffer_end, &dims, buffer_out, out_size, buffer_tmp);

    printf("#Value,Value after comp-decomp,Difference\n");
    for (size_t i = 0; i < count; ++i) {
        printf("%f,%f,%f\n", buffer_in[i], buffer_end[i], buffer_end[i] - buffer_in[i]);
    }

    printf("#Buffer size,Buffer size after compression\n");
    printf("%lu,%lu\n", uncompressed_size, out_size);

    double error_sum = 0.0;
    for (size_t i = 0; i < count; i++) {
        error_sum += buffer_end[i] - buffer_in[i];
    }
    printf("#Error sum\n");
    printf("%f\n", error_sum);

    free(buffer_in);
    free(buffer_out);
    free(buffer_tmp);
    free(buffer_end);

    //scil_destroy_context(context);

    return 0;
}
