// This file is part of SCIL.
//
// SCIL is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SCIL is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with SCIL.  If not, see <http://www.gnu.org/licenses/>.

// This file is the skeleton for each simple test.
#include <scil.h>
#include <scil-error.h>

#include <assert.h>
#include <stdio.h>
#include <string.h>

#define SUCCESS 0

static double data[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
static double data_check[] = {0,0,0,0,0,0,0,0,0,0};
static scil_context_p ctx;
static scilPr_user_hints_t hints;
static scil_dims dims;
static size_t size;
static byte * buff;
static byte * tmpBuff;


void test(char * name, size_t expected_size, int check_compressed_output){
  int ret;
  printf("Running %s\n", name);
  hints.force_compression_methods = name;
  ret = scil_create_compression_context(& ctx, SCIL_TYPE_DOUBLE, 0, NULL, &hints);
  assert(ret == SCIL_NO_ERR);

  size_t out_size = -1;
  ret = scil_compress(buff, size, data, & dims, & out_size,  ctx);
  printf("%d %lld\n", ret, (long long) out_size);
  assert(ret == SCIL_NO_ERR && "ERROR COMPRESSION");
  assert(out_size == expected_size);

  if(check_compressed_output){
    assert( memcmp(& buff[1], data, sizeof(data)) == 0);
  }

  ret = scil_destroy_compression_context(& ctx);
  assert(ret == SCIL_NO_ERR);

  memset(data_check, 0, sizeof(data_check));
  ret = scil_decompress(SCIL_TYPE_DOUBLE, data_check, & dims, buff, out_size, tmpBuff);
  assert(ret == SCIL_NO_ERR && "ERROR DECOMPRESSION");

  int compare = memcmp(data_check, data, sizeof(data)) == 0;
  if(! compare){
    for(int i=0; i < 10; i++){
      printf("%f ", data_check[i]);
    }
    printf("\n");
  }

  assert( compare );
}

int main(){
  scilPr_initialize_user_hints(& hints);
  hints.absolute_tolerance = SCIL_ACCURACY_DBL_FINEST;

  scil_init_dims_1d(& dims, 10);
  size = scil_compress_buffer_size_bound(SCIL_TYPE_DOUBLE, & dims);
  buff = malloc(size*10);
  tmpBuff = malloc(size*10);
  assert(sizeof(data) == 80);

  test("dummy-precond", 87, 1);
  test("dummy-precond,dummy-precond", 93, 1);
  test("dummy-precond,dummy-precond,dummy-precond,dummy-precond", 105, 1);

  test("dummy-precond,lz4", 58, 0);
  test("dummy-precond,dummy-precond,lz4", 64, 0);
  test("dummy-precond,dummy-precond,dummy-precond,lz4", 71, 0);

  test("lz4", 56, 0);
  test("zfp-abstol", 98, 0);

  test("zfp-abstol,lz4", 47, 0);

  test("dummy-precond,zfp-abstol", 104, 0);
  test("dummy-precond,dummy-precond,zfp-abstol", 110, 0);

  test("dummy-precond,zfp-abstol,lz4", 49, 0);
  test("dummy-precond,dummy-precond,zfp-abstol,lz4", 55, 0);

  free(buff);

  printf("OK\n");
  return SUCCESS;
}
