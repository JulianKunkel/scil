//#include <stdint.h>

#include <scil.h>
#include <scil-error.h>
#include <scil-util.h>

#include <basic-patterns.h>

int main(void)
{
    const size_t count = 100;

    double* source = (double*)scilU_safe_malloc(count * sizeof(double));

    scil_dims_t dims;
    scil_dims_initialize_1d(&dims, count);

    scil_pattern_rnd.create(source, &dims, -100.0f, 100.0f, 0.0f, 0.0f, 10);

    size_t dest_size = scil_get_compressed_data_size_limit(&dims, SCIL_TYPE_DOUBLE);
    byte* dest       = (byte*)scilU_safe_malloc(dest_size);

    scil_user_hints_t hints;
    scil_user_hints_initialize(&hints);
    hints.force_compression_methods = "1";
    hints.absolute_tolerance        = 1e-300;

    scil_context_t* ctx;
    scil_context_create(&ctx, SCIL_TYPE_DOUBLE, 0, NULL, &hints);

    int ret = scil_compress(dest, dest_size, source, &dims, &dest_size, ctx);
    printf("%d %d\n", ret, SCIL_PRECISION_ERR);

    free(source);
    free(dest);
    free(ctx);

    return ret != SCIL_PRECISION_ERR;
}
