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

#include <string.h>

#include <scil-internal.h>
#include <scil-algo-chooser.h>

static float determine_randomness_float(float* source, size_t count){
  // do sth...
  return 0;
}

static float determine_randomness_double(double* source, size_t count){
  return 0;
}


void scil_compression_algo_chooser(void*restrict source, scil_dims* dims, scil_context_p ctx){
  scil_compression_chain_t * chain = & ctx->chain;
  int ret;

  // at the moment we only set the compression algorith once
  if (chain->total_size != 0){
    return;
  }
  char * chainEnv = getenv("SCIL_FORCE_COMPRESSION_CHAIN");
  if (chainEnv != NULL){
    if (strcmp(chainEnv, "lossless") == 0){
      ctx->lossless_compression_needed = 1;
    }else{
      ret = scilI_parse_compression_algorithms(chain, chainEnv);
      if (ret != SCIL_NO_ERR){
        critical("[SCIL] The environment variable SCIL_FORCE_COMPRESSION_CHAIN is invalid with \"%s\"\n", chainEnv);
      }
      return;
    }
  }
  const size_t count = scil_get_data_count(dims);

  if (count < 10){
    // always use memcopy for small data
    ret = scilI_parse_compression_algorithms(chain, "memcopy");
    return;
  }

  float r;
  switch(ctx->datatype){
    case SCIL_TYPE_FLOAT:
      r = determine_randomness_float((float*)source, count);
      break;
    case SCIL_TYPE_DOUBLE:
      r = determine_randomness_double((double*)source, count);
      break;
  }

  if (ctx->lossless_compression_needed){
      // we cannot compress because data must be accurate!
  }
  // TODO: pick the best algorithm for the settings given in ctx...

  if (r > 99){
    ret = scilI_parse_compression_algorithms(chain, "memcopy");
  }else{
    ret = scilI_parse_compression_algorithms(chain, "lz4");
  }
  assert(ret == SCIL_NO_ERR);
}
