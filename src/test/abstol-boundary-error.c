#include <scil-util.h>
#include <scil.h>

#include <scil-internal.h>
#include <scil-patterns.h>

int main(void)
{
    const size_t count = 10;

    double source[] = {0, 1, 2, 3,4,5,6,7,8,9};

    scil_dims dims;
    scil_init_dims_1d(&dims, count);

    size_t dest_size = scil_compress_buffer_size_bound(SCIL_TYPE_DOUBLE, &dims);
    byte* dest       = (byte*) SAFE_MALLOC(dest_size);

    scil_user_hints_t hints;
    scilPr_initialize_user_hints(&hints);
    hints.force_compression_methods = "abstol";
    hints.absolute_tolerance        = 1;

    scil_context_p ctx;
    size_t compressed_size;
    scilPr_create_context(&ctx, SCIL_TYPE_DOUBLE, 0, NULL, &hints);

    int ret = scil_compress(dest, dest_size, source, &dims, &compressed_size, ctx);
    // scil_decompress();

    scil_user_hints_t out_accuracy;
    ret = scil_validate_compression(SCIL_TYPE_DOUBLE, source, & dims, dest, compressed_size,  ctx, & out_accuracy);

    scilPr_print_user_hints(& out_accuracy);

    assert( ret == 0);

    free(dest);
    free(ctx);

    return ret;
}
