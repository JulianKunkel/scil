#include <scil.h>
#include <scil-error.h>
#include <scil-util.h>

#include <basic-patterns.h>

int main(void)
{
    const size_t count = 100;

    double* source = (double*)SAFE_MALLOC(count * sizeof(double));

    scil_dims dims;
    scil_init_dims_1d(&dims, count);

    scil_pattern_rnd.create(&dims, source, -100.0f, 100.0f, 0.0f, 0.0f);

    size_t dest_size = scil_compress_buffer_size_bound(SCIL_TYPE_DOUBLE, &dims);
    byte* dest       = (byte*)SAFE_MALLOC(dest_size);

    scilPr_user_hints_t hints;
    scilPr_initialize_user_hints(&hints);
    hints.force_compression_methods = "3";
    hints.significant_bits          = 64;

    scil_context_p ctx;
    scil_create_compression_context(&ctx, SCIL_TYPE_DOUBLE, 0, NULL, &hints);

    int ret = scil_compress(dest, dest_size, source, &dims, &dest_size, ctx);
    printf("%d %d\n", ret, SCIL_PRECISION_ERR);

    free(source);
    free(dest);
    free(ctx);

    return ret != SCIL_PRECISION_ERR;
}
