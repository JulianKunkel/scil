//#include <stdint.h>

#include <scil.h>
#include <scil-error.h>
#include <scil-internal.h>
#include <scil-patterns.h>
#include <scil-util.h>

int main(void)
{
    const size_t count = 100;

    float* source = (float*)SAFE_MALLOC(count * sizeof(float));

    scil_dims_t dims;
    scilPr_initialize_dims_1d(&dims, count);

    scilPa_create_pattern(source, SCIL_TYPE_FLOAT, &dims, "random", -100.0f, 100.0f, 0.0f, 0.0f);

    size_t dest_size = scilPr_get_compressed_data_size_limit(&dims, SCIL_TYPE_FLOAT);
    byte* dest       = (byte*)SAFE_MALLOC(dest_size);

    scil_user_hints_t hints;
    scilPr_initialize_user_hints(&hints);
    hints.force_compression_methods = "1";
    hints.absolute_tolerance        = 1e-100;

    scil_context_t* ctx;
    scilPr_create_context(&ctx, SCIL_TYPE_FLOAT, 0, NULL, &hints);

    //printf("%s\n", ctx->chain.byte_compressor->name);
    //printf("%s\n", ctx->chain.data_compressor->name);

    int ret = scil_compress(dest, dest_size, source, &dims, &dest_size, ctx);
    printf("%d %d\n", ret, SCIL_PRECISION_ERR);

    free(source);
    free(dest);
    free(ctx);

    return ret != SCIL_PRECISION_ERR;
}
