//#include <stdint.h>

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

    scil_pattern_t_rnd.create(source, &dims, -100.0f, 100.0f, 0.0f, 0.0f);

    size_t dest_size = scil_compress_buffer_size_bound(SCIL_TYPE_DOUBLE, &dims);
    byte* dest       = (byte*)SAFE_MALLOC(dest_size);

    scil_user_hints_t hints;
    scilPr_initialize_user_hints(&hints);
    hints.force_compression_methods = "1";
    hints.absolute_tolerance        = 1e-300;

    scil_context_t* ctx;
    scilPr_create_context(&ctx, SCIL_TYPE_DOUBLE, 0, NULL, &hints);

    int ret = scil_compress(dest, dest_size, source, &dims, &dest_size, ctx);
    printf("%d %d\n", ret, SCIL_PRECISION_ERR);

    free(source);
    free(dest);
    free(ctx);

    return ret != SCIL_PRECISION_ERR;
}
