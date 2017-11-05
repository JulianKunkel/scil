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

#include <stdio.h>
#include <assert.h>
#include <string.h>

#include <scil-util.h>

#pragma GCC diagnostic ignored "-Wunused-parameter"

static void printVarDimsOffsets(double* data, const scil_dims_t* pos, const scil_dims_t* size, int* iter, const void* user_ptr){
  scilU_print_dims(*pos);
}

static void initVarDims(double* data, const scil_dims_t* pos, const scil_dims_t* size, int* iter, const void* user_ptr){
  double val = 0;
  for(int i=0; i < pos->dims; i++){
    val = val * 10 + pos->length[i];
  }
  data[scilU_data_pos(pos, size)] = val;
}

static void addVarDims(double* data, const scil_dims_t* pos, const scil_dims_t* size, int* iter, const void* user_ptr){
  double* val = (double*) user_ptr;
  *val += data[scilU_data_pos(pos, size)];
}

static void printVarDims(double* data, const scil_dims_t* pos, const scil_dims_t* size, int* iter, const void* user_ptr){
  printf("%.0f ", data[scilU_data_pos(pos, size)]);
  size_t x = pos->dims - 1;
  if (pos->length[x] == size->length[x] - 1){
    printf("\n");
    if( x > 1){
      if (pos->length[x - 1] == size->length[x - 1] - 1){
        printf("\n");
        if( x > 2){
          if (pos->length[x - 2] == size->length[x - 2] - 1){
            printf("\n");
          }
        }
      }
    }
  }
}

int main(){
  scil_dims_t dims;
  double * data = malloc(sizeof(double) * 10 * 10 * 10);

  scil_dims_t offset;

  scil_dims_initialize_3d(& dims, 10,10,10);
  scil_dims_initialize_4d(& offset, 0,0,0,0);
  scilU_iter(data, &dims, &offset, &dims, NULL, & initVarDims, NULL );
  scilU_iter(data, &dims, &offset, &dims, NULL, & printVarDims, NULL );

  scil_dims_initialize_1d(& dims, 10);
  scilU_iter(data, &dims, &offset, &dims, NULL, & printVarDimsOffsets, NULL );

  scil_dims_initialize_2d(& dims, 3, 2);
  scilU_iter(data, &dims, &offset, &dims, NULL, & printVarDimsOffsets, NULL );

  scil_dims_initialize_3d(& dims, 3, 2, 3);
  scilU_iter(data, &dims, &offset, &dims, NULL, & printVarDimsOffsets, NULL );

  scil_dims_initialize_3d(& dims, 3, 1, 2);
  scilU_iter(data, &dims, &offset, &dims, NULL, & printVarDimsOffsets, NULL );
  scil_dims_initialize_4d(& dims, 3, 1, 2, 1);
  scilU_iter(data, &dims, &offset, &dims, NULL, & printVarDimsOffsets, NULL );

  printf("\nOther scilU_iter steps\n");
  scil_dims_initialize_1d(& dims, 10);
  scilU_iter(data, &dims, &offset, &dims, (int[]){3}, & printVarDimsOffsets, NULL);
  scil_dims_initialize_2d(& dims, 10, 10);
  scilU_iter(data, &dims, &offset, &dims, (int[]){3,3}, & printVarDimsOffsets, NULL );
  scil_dims_initialize_3d(& dims, 10, 10, 10);
  scilU_iter(data, &dims, &offset, &dims, (int[]){6,6,6}, & printVarDimsOffsets, NULL);

  double value = 0;
  scil_dims_initialize_3d(& dims, 10,10,10);
  scilU_iter(data, &dims, &offset, &dims, NULL, & addVarDims, &value );
  printf("Sum: %.0f\n", value);
  assert(scilU_double_equal(value, 499500.0));

  free(data);

  return 0;
}
