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
#include <float.h>

#include <scil-internal.h>
#include <scil-algo-chooser.h>
#include <scil-config.h>

#include <algo/lz4fast.h>


static float determine_randomness(void* source, size_t count){
  // We may want to use https://en.wikipedia.org/wiki/Randomness_tests
  byte buffer[15000];
  size_t out_size = 15000;
  size_t in_size = 10000;
  if (count < in_size){
    in_size = count;
  }
  int ret = scil_lz4fast_compress(NULL, buffer, &out_size, source, in_size);
  if (ret == 0){
    double rnd = out_size * 100.0 / in_size ;
    return rnd;
  }else{
    critical("lz4fast error to determine_randomness %d\n", ret);
  }
}

void scil_compression_algo_chooser_init(){
  char * filename = getenv("SCIL_SYSTEM_CHARACTERISTICS_FILE");
  if(filename == NULL){
    filename = SYSTEM_CONFIGURATION_FILE;
  }
  FILE * data = fopen("scil.conf", "r");
  if (data == NULL){
    data = fopen(filename, "r");
    if(data == NULL){
      warn("Could not open configuration file %s\n", filename);
      return;
    }
  }

  

  fclose(data);
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
        critical("The environment variable SCIL_FORCE_COMPRESSION_CHAIN is invalid with \"%s\"\n", chainEnv);
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

  float r = determine_randomness(source, count);
  if (ctx->lossless_compression_needed){
      // we cannot compress because data must be accurate!
  }
  // TODO: pick the best algorithm for the settings given in ctx...

  if (r > 95){
    ret = scilI_parse_compression_algorithms(chain, "memcopy");
  }else{
    ret = scilI_parse_compression_algorithms(chain, "lz4");
  }
  assert(ret == SCIL_NO_ERR);
}


/*
// determine min, max, mean and stdev
double mean = 0;
double mn = DBL_MAX;
double mx = DBL_MIN;
for(int i=0; i < count; i++){
  mean += source[i];
  mx = max(mx, source[i]);
  mn = min(mn, source[i]);
}
mean /= SAMPLES;

double spread = mx - mn;

double stddev = 0;
for(int i=0; i < count; i++){
  stddev += (source[i]-mean)*(source[i]-mean);
}
printf("%f %f stddev %f\n", mn, mx, stddev/spread/spread/count/count);
*/
