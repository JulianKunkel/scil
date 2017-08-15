#include <stdio.h>
#include <time.h>

#include <scil.h>

double expected_lower_value(double orig_value) {
    int int_value = (int)orig_value;
    switch(int_value) {
        case 1:
        case 2:
        case 3:
        case 4:
        case 6:
        case 8:
          return orig_value;
        case 5:
          return 4.0;
        case 7:
          return 6.0;
        case 9:
          return 8.0;
        case 10:
          return 8.0;
    }
}

double expected_upper_value(double orig_value) {
    int int_value = (int)orig_value;
    switch(int_value) {
        case 1:
        case 2:
        case 3:
        case 4:
        case 6:
        case 8:
          return orig_value;
        case 5:
          return 6.0;
        case 7:
          return 8.0;
        case 9:
          return 12.0;
        case 10:
          return 12.0;
    }
}

int main(void){

    size_t count = 10;

    scil_user_hints_t hints;
    scilPr_initialize_user_hints(&hints);
    hints.significant_bits = 2;
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
    printf("#Expected lower value,Expected upper value,Value after comp-decomp,Status\n");
    for (size_t i = 0; i < count; ++i) {
        double lower = expected_lower_value(buffer_in[i]); 
        double upper = expected_upper_value(buffer_in[i]); 
        if ((buffer_end[i] != lower) && (buffer_end[i] != upper)) {
            printf("%f,%f,%f,Error\n", lower, upper, buffer_end[i]);
            errors++;
        }
        else
            printf("%f,%f,%f,Ok\n", lower, upper, buffer_end[i]);
    }

    free(buffer_in);
    free(buffer_out);
    free(buffer_tmp);
    free(buffer_end);

    //scilPr_destroy_context(context);

    return errors;
}
