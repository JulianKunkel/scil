#include <string.h>

#include <scil-util.h>
#include <scil.h>

#include <scil-debug.h>


int main(void)
{
    const size_t count = 10;

    double source[] = {0, 1, 2, 3,4,5,6,7,8,9};

    scil_dims_t dims;
    scil_dims_initialize_1d(&dims, count);

    size_t dest_size = scil_get_compressed_data_size_limit(&dims, SCIL_TYPE_DOUBLE);
    byte* dest       = (byte*) scilU_safe_malloc(dest_size);

    scil_user_hints_t hints;
    scil_user_hints_initialize(&hints);
    hints.force_compression_methods = "abstol";
    hints.absolute_tolerance        = 1;

    scil_context_t* ctx;
    size_t compressed_size;
    scil_context_create(&ctx, SCIL_TYPE_DOUBLE, 0, NULL, &hints);

    int ret = scil_compress(dest, dest_size, source, &dims, &compressed_size, ctx);
    // scil_decompress();

    byte* data_out        = (byte*)malloc(dest_size);
    memset(data_out, -1, dest_size);
    ret = scil_decompress(SCIL_TYPE_DOUBLE, data_out, & dims, dest, compressed_size, &data_out[dest_size / 2]);

    for(size_t i=0; i < count; i++){
      printf("%f - %f\n", source[i], ((double*)data_out)[i]);
    }

    scil_user_hints_t out_accuracy;
    scil_validate_params_t out_validation;
    ret = scil_validate_compression(SCIL_TYPE_DOUBLE, source, & dims, dest, compressed_size,  ctx, & out_accuracy, & out_validation);

    scil_user_hints_print(& out_accuracy);

    assert( ret == 0);

    free(dest);
    free(ctx);

    return ret;
}
