#include <stdio.h>
#include <time.h>

#include <scil.h>

int main(void){

    size_t count = 10;

    scil_user_hints_t hints;
    scilPr_initialize_user_hints(&hints);
    hints.significant_bits = 4;
    hints.force_compression_methods = "3";

    scil_context_t* context;
    scilPr_create_context(&context, SCIL_TYPE_DOUBLE, 0, NULL, &hints);

    scil_dims_t dims;
    scilPr_initialize_dims_1d(&dims, count);

    size_t uncompressed_size = scilPr_get_dims_size(&dims, SCIL_TYPE_DOUBLE);
    size_t compressed_size   = scilPr_get_compressed_data_size_limit(&dims, SCIL_TYPE_DOUBLE);

    double* buffer_in  = (double*)malloc(uncompressed_size);
    byte* buffer_out   = (byte*)malloc(compressed_size);
    byte* buffer_tmp   = (byte*)malloc(compressed_size / 2);
    double* buffer_end = (double*)malloc(uncompressed_size);

    for(size_t i = 0; i < count; ++i){
        buffer_in[i] = (double)(i + 1);
    }

    size_t out_size;
    scil_compress(buffer_out, compressed_size, buffer_in, &dims, &out_size, context);
    scil_decompress(SCIL_TYPE_DOUBLE, buffer_end, &dims, buffer_out, out_size, buffer_tmp);

    int errors = 0;
    printf("#Expected value,Value after comp-decomp,Status\n");
    for (size_t i = 0; i < count; ++i) {
        double expected = buffer_in[i]; 
        if (buffer_end[i] != expected) {
            printf("%f,%f,Error\n", expected, buffer_end[i]);
            errors++;
        }
        else
            printf("%f,%f,Ok\n", expected, buffer_end[i]);
    }

    free(buffer_in);
    free(buffer_out);
    free(buffer_tmp);
    free(buffer_end);

    //scilPr_destroy_context(context);

    return errors;
}
