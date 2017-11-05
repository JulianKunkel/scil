//#include <stdint.h>

#include <scil.h>
#include <scil-error.h>
#include <scil-debug.h>
#include <scil-patterns.h>
#include <scil-util.h>

int main(void)
{
    const size_t count = 100;

    float* source = (float*)scilU_safe_malloc(count * sizeof(float));

    scil_dims_t dims;
    scil_dims_initialize_1d(&dims, count);

    scilP_create_pattern(source, SCIL_TYPE_FLOAT, &dims, "random", -100.0f, 100.0f, 0.0f, 0.0f);

    size_t dest_size = scil_get_compressed_data_size_limit(&dims, SCIL_TYPE_FLOAT);
    byte* dest       = (byte*)scilU_safe_malloc(dest_size);

    scil_user_hints_t hints;
    scil_user_hints_initialize(&hints);
    hints.force_compression_methods = "1";
    hints.absolute_tolerance        = 1e-100;

    scil_context_t* ctx;
    scil_context_create(&ctx, SCIL_TYPE_FLOAT, 0, NULL, &hints);

    //printf("%s\n", ctx->chain.byte_compressor->name);
    //printf("%s\n", ctx->chain.data_compressor->name);

    int ret = scil_compress(dest, dest_size, source, &dims, &dest_size, ctx);
    printf("%d %d\n", ret, SCIL_PRECISION_ERR);

    free(source);
    free(dest);
    free(ctx);

    return ret != SCIL_PRECISION_ERR;
}
