// This file is part of SCIL.
//
// SCIL is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SCIL is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FpITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with SCIL.  If not, see <http://www.gnu.org/licenses/>.

/* Remember: due to the machine epsilon of the floating-point data representation,
the maximum decompression error would be slightly larger than the specified error bound in some cases.*/

#include <assert.h>
#include <math.h>
#include <string.h>

#include <sz.h>

#include <algo/algo-sz.h>
#include <scil-util.h>

static struct sz_params * params = NULL;

static void init_sz_if_needed(){
  if (params) return;
  params = malloc(sizeof(struct sz_params));
  struct sz_params * p = params;
  memset(p, -1, sizeof(struct sz_params));
  p->dataEndianType = LITTLE_ENDIAN_DATA;
  p->max_quant_intervals = 65536;
  p->quantization_intervals = 0;
  p->layers = 1;
  p->sampleDistance = 100;
  p->predThreshold = 0.97;
  p->offset = 0;
  p->szMode = SZ_BEST_COMPRESSION;
  //p.gzipMode = Z_BEST_SPEED;
  p->errorBoundMode = ABS; //ABS_AND_REL
  p->absErrBound = 0.0001;
  p->relBoundRatio = 0.001;
  p->pw_relBoundRatio = 0.000001;
  p->segment_size = 32;

  SZ_Init_Params(p);
}

//Repeat for each data type
//Supported datatypes: double float

int scil_sz_compress_<DATATYPE>(const scil_context_t* ctx,
                                    byte* restrict dest,
                                    size_t* restrict dest_size,
                                    <DATATYPE>* restrict source,
                                    const scil_dims_t* dims){
  init_sz_if_needed();
  size_t size = 0;
  double abstol = ctx->hints.absolute_tolerance;
  double reltol = ctx->hints.relative_tolerance_percent / 100.0;
  int mode = 0;
  if (abstol > 0.0 && reltol > 0.0){
    //assert(0 && "Not supported!");
    // TODO: remember rel tolerance is based on the delta: max-min for SZ but for us it is based on max
    mode = ABS_AND_REL;
  }else if(reltol > 0.0 ){
    // TODO: remember rel tolerance is based on the delta: max-min for SZ but for us it is based on max
    mode = REL;
  }else{
    mode = ABS;
  }
  //printf("Running SZ: with %d %f %f\n", mode, abstol, reltol);

  int ret;
  ret = SZ_compress_args2(SZ_<DATATYPE_UPPER>, source, dest, & size, mode, abstol, reltol, 0.0, 0, 0, dims->length[3], dims->length[2], dims->length[1], dims->length[0]);
  //printf("Returns: %d\n", size);
  if (ret == 0){
    *dest_size = size;
    return SCIL_NO_ERR;
  }else{
    printf("SZ CError: %d\n", ret);
    return SCIL_UNKNOWN_ERR;
  }
}

int scil_sz_decompress_<DATATYPE>(<DATATYPE>* restrict dest,
                                      scil_dims_t* dims,
                                      byte* restrict source,
                                      size_t source_size){
  init_sz_if_needed();
  int size = (int) source_size;
  //printf("Decompress %d %d\n", size, dims->length[0]);
  int elems = SZ_decompress_args(SZ_<DATATYPE_UPPER>, source, size, (void*) dest, 0, dims->length[3], dims->length[2], dims->length[1], dims->length[0]);

  if (elems < 0){
    printf("SZ DError: %d\n", elems);
    return SCIL_UNKNOWN_ERR;
  }
  size_t count = scil_dims_get_count(dims);
  if (count != (size_t) elems){
    printf("SZ Short read %d instead of %d!\n", elems, (int) count);
    return SCIL_UNKNOWN_ERR;
  }

  return SCIL_NO_ERR;
}
// End repeat



scilU_algorithm_t algo_sz = {
    .c.DNtype = {
        CREATE_INITIALIZER(scil_sz)
    },
    "sz",
    13,
    SCIL_COMPRESSOR_TYPE_DATATYPES,
    1
};
