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
#include <scil-util.h>

#include <assert.h>
#include <stdio.h>

#define SUCCESS 0

int main(){

  scil_context_t* ctx;
  scil_user_hints_t hints;
  scil_dims_t dims;
  int ret;

  scil_user_hints_initialize(& hints);
  hints.force_compression_methods = "dummy-precond";
  ret = scil_context_create(&ctx, SCIL_TYPE_DOUBLE, 0, NULL, &hints);
  assert(ret == SCIL_NO_ERR);

  size_t size;
  byte * buff;

  scil_dims_initialize_1d(& dims, 0);
  size = scil_get_compressed_data_size_limit(&dims, SCIL_TYPE_DOUBLE);
  buff = malloc(size);

  double data[] = {1};

  size_t out_size = -1;
  ret = scil_compress(buff, size, data, & dims, & out_size,  ctx);
  assert(ret == SCIL_NO_ERR);
  assert(out_size == 1);

  scil_destroy_context(ctx);

  free(buff);

  printf("OK\n");
  return SUCCESS;
}
