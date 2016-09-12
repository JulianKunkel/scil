#include <scil.h>
#include <scil-error.h>
#include <scil-util.h>

#include <scil-internal.h>
#include <scil-patterns.h>

int main()
{
    const size_t count = 100;

    float* source = (float*)SAFE_MALLOC(count * sizeof(float));

    scil_dims dims;
    scil_init_dims_1d(&dims, count);

    scilP_create_pattern_float(&dims, source, "random", -100.0f, 100.0f, 0.0f, 0.0f);

    size_t dest_size = scil_compress_buffer_size_bound(SCIL_TYPE_FLOAT, &dims);
    byte* dest       = (byte*)SAFE_MALLOC(dest_size);

    scil_hints hints;
    scil_init_hints(&hints);
    hints.force_compression_methods = "3";
    hints.significant_bits          = 32;

    scil_context_p ctx;
    scil_create_compression_context(&ctx, SCIL_TYPE_FLOAT, 0, NULL, &hints);

    //printf("%s\n", ctx->chain.byte_compressor->name);
    //printf("%s\n", ctx->chain.data_compressor->name);

    int ret = scil_compress(dest, dest_size, source, &dims, &dest_size, ctx);
    printf("%d %d\n", ret, SCIL_PRECISION_ERR);

    free(source);
    free(dest);
    free(ctx);

    return ret != SCIL_PRECISION_ERR;
}
