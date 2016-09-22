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

#include <scil-algo-chooser.h>

#include <scil-compression-chain.h>
#include <scil-config.h>
#include <scil-data-characteristics.h>
#include <scil-hardware-limits.h>
#include <scil-internal.h>

#include <stdio.h>
#include <string.h>

typedef struct{
  scil_compression_chain_t chain;
  float randomness;
  float c_speed;
  float d_speed;
  float ratio;
} config_file_entry_t;

static config_file_entry_t * config_list;
static int config_list_size = 0;

static config_file_entry_t ** config_list_lossless;
static int config_list_lossless_size = 0;

static void parse_losless_list(){
  config_list_lossless = (config_file_entry_t**) malloc(sizeof(void*)* config_list_size);

  for(int i=0; i < config_list_size;i++){
    if (! config_list[i].chain.is_lossy){
      config_list_lossless[config_list_lossless_size] = & config_list[i];
      config_list_lossless_size++;
    }
  }
  debug("Configuration found %d entries which are lossless\n", config_list_lossless_size);
  config_list_lossless = (config_file_entry_t**) realloc(config_list_lossless, config_list_lossless_size * sizeof(void*));
}

void scilI_compression_algo_chooser_init(){
  int ret;
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

  // File format is in CSV, see "dev/scil.conf" for an example->
  char * buff = malloc(1024);
  int config_list_capacity = 100;
  config_list = (config_file_entry_t*) malloc(sizeof(config_file_entry_t)* config_list_capacity); // up to 1k entries

  while(true){
    size_t length = 1024;
    ssize_t linelength;
    linelength = getline(&buff, & length, data);
    if(linelength == -1){
      break;
    }
    if(buff[0] == '#' || strlen(buff) < 5){
      // ignore comments
      continue;
    }
    if(buff[strlen(buff)-1]=='\n') buff[strlen(buff)-1] = 0;

    if(buff[0] == '!'){
      // hardware limit
      char * pos = strstr(buff, " ");
      if(pos == NULL){
        warn("Invalid configuration line \"%s\"\n", buff);
        continue;
      }
      *pos = 0;
      ret = scilI_add_hardware_limit(& buff[1], & pos[1]);
      if (ret != SCIL_NO_ERR){
        warn("Invalid configuration line \"%s\"\n", buff);
      }
      continue;
    }

    config_file_entry_t * e = & config_list[config_list_size];
    char name[100];
    int tokens = sscanf(buff, "%f; %s %f; %f; %f", & e->randomness, name, & e->c_speed, & e->d_speed, & e->ratio);
    if( tokens != 5){
      warn("Parsing configuration line \"%s\" returned an error after token %d\n", buff, tokens);
      continue;
    }
    name[strlen(name)-1] = 0;
    ret = scilI_parse_compression_algorithms(& e->chain, name);
    if (ret != SCIL_NO_ERR){
      warn("Parsing configuration line \"%s\"; could not parse compressor chain \"%s\"\n", buff, name);
      continue;
    }
    debug("Configuration line %.3f; %s; %.1f; %.1f; %.3f\n", (double) e->randomness, name, (double) e->c_speed, (double) e->d_speed, (double) e->ratio);

    config_list_size++;
    if(config_list_size >= config_list_capacity){
      config_list_capacity *= 5;
      config_list = (config_file_entry_t*) realloc(config_list, config_list_capacity * sizeof(config_file_entry_t));
      debug("Configuration list increasing size to %d\n", config_list_capacity);
    }
  }
  config_list = (config_file_entry_t*) realloc(config_list, config_list_size * sizeof(config_file_entry_t));
  free(buff);
  fclose(data);

  debug("Configuration, parsed %d lines\n", config_list_size);

  parse_losless_list();
}

void scilI_compression_algo_chooser(void*restrict source, scil_dims* dims, scil_context_p ctx){
  scil_compression_chain_t * chain = &ctx->chain;
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

  size_t out_size = 15000;
  byte buffer[15000];
  size_t in_size = 10000;
  if (count < in_size){
    in_size = count;
  }

  float r = scilI_determine_randomness(source, in_size, buffer, out_size);
  if (ctx->lossless_compression_needed){
      // we can only select byte compressors compress because data must be accurate!
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
