#include <scil-util.h>
#include <scil.h>

#include <scil-internal.h>
#include <scil-patterns.h>

const size_t count = 10;

void test(int datatype, void * source){
  int ret;
  scil_dims_t dims;
  scilPr_initialize_dims_1d(&dims, count);
  size_t dest_size = scilPr_get_compressed_data_size_limit(&dims, datatype);
  byte* dest       = (byte*) SAFE_MALLOC(dest_size);

  scil_user_hints_t hints;
  scilPr_initialize_user_hints(&hints);

  hints.absolute_tolerance  = 1;

  scil_context_t* ctx;
  size_t compressed_size;
  scilPr_create_context(&ctx, datatype, 0, NULL, &hints);

  ret = scil_compress(dest, dest_size, source, &dims, &compressed_size, ctx);

  printf("Compressed size: %ld\n", compressed_size);

  scil_user_hints_t out_accuracy;
  ret = scil_validate_compression(datatype, source, & dims, dest, compressed_size,  ctx, & out_accuracy);

  scilPr_print_user_hints(& out_accuracy);

  assert( ret == 0);

  free(dest);
  free(ctx);
}

int main(void)
{
  {
    printf("String\n");
    char * source = "this\0is\0a\0";
    test(SCIL_TYPE_STRING, source);
  }
  {
    double source[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    test(SCIL_TYPE_DOUBLE, source);
  }
  {
    float source[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    test(SCIL_TYPE_FLOAT, source);
  }
  {
    int8_t source[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    test(SCIL_TYPE_INT8, source);
  }
  {
    int16_t source[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    test(SCIL_TYPE_INT16, source);
  }
  {
    int32_t source[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    test(SCIL_TYPE_INT32, source);
  }
  {
    int64_t source[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    test(SCIL_TYPE_INT64, source);
  }
  return 0;
}
