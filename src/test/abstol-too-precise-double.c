//#include <stdint.h>

#include <scil-util.h>
#include <scil.h>

#include <basic-patterns.h>

int main(int argc, char** argv)
{
    const size_t count = 100;

    double* source = (double*)SAFE_MALLOC(count * sizeof(double));

    scil_dims dims;
    scil_init_dims_1d(&dims, count);

    scil_pattern_rnd.create(&dims, source, -100.0f, 100.0f, 0.0f);

    size_t dest_size = scil_compress_buffer_size_bound(SCIL_TYPE_DOUBLE, &dims);
    byte* dest       = (byte*)SAFE_MALLOC(dest_size);

    scil_context_p ctx;
    scil_hints hints;
    hints.absolute_tolerance        = 1e-100;
    hints.force_compression_methods = "1";
    scil_create_compression_context(&ctx, SCIL_TYPE_DOUBLE, &hints);

    int ret = scil_compress(dest, dest_size, source, &dims, &dest_size, ctx);

    return ret != SCIL_PRECISION_ERR;
}
