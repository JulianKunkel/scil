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

#include <scil-config.h>

#include <scil-context-impl.h>
#include <scil-algo-chooser.h>
#include <scil-compression-chain.h>
#include <scil-data-characteristics.h>
#include <scil-error.h>
#include <scil-hardware-limits.h>
#include <scil-debug.h>
#include <scil-decision-tree.h>

#include <stdio.h>
#include <string.h>

typedef struct {
  scil_compression_chain_t chain;
  float randomness;
  float c_speed;
  float d_speed;
  float ratio;
} config_file_entry_t;

static config_file_entry_t *config_list;
static int config_list_size = 0;

static config_file_entry_t **config_list_lossless;
static int config_list_lossless_size = 0;

static void parse_losless_list() {
  config_list_lossless = (config_file_entry_t **) malloc(sizeof(void *) * config_list_size);

  for (int i = 0; i < config_list_size; i++) {
    if (!config_list[i].chain.is_lossy) {
      config_list_lossless[config_list_lossless_size] = &config_list[i];
      config_list_lossless_size++;
    }
  }
  debug("Configuration found %d entries which are lossless\n", config_list_lossless_size);
  config_list_lossless =
      (config_file_entry_t **) realloc(config_list_lossless, config_list_lossless_size * sizeof(void *));
}

static char** parse_array_chars(char* config){
  int count_elements = 0;
  for(unsigned int i=0; i<strlen(config); ++i){
    if(config[i] == ';') ++count_elements;
  }
  count_elements += 1;
  char** tree_classes = malloc(sizeof(char*)*count_elements);

  char* item = strtok(config, ";");
  int class_index = 0;
  while(item != NULL){
    tree_classes[class_index] = malloc(sizeof(char*)*(strlen(item)+1));
    strcpy(tree_classes[class_index], item);
    item = strtok(NULL, ";");
    //warn("Item %s\n", tree_classes[class_index]);
    ++class_index;
  }
  return tree_classes;
}


static double* parse_array_double(char* config){
  int count_elements = 0;
  for(unsigned int i=0; i<strlen(config); ++i){
    if(config[i] == ';') ++count_elements;
  }
  count_elements += 1;
  double* tree_classes = malloc(sizeof(double*)*count_elements);

  char* item = strtok(config, ";");
  int class_index = 0;
  while(item != NULL){
    tree_classes[class_index] = atof(item);
    item = strtok(NULL, ";");
    ++class_index;
  }
  return tree_classes;
}

static int* parse_array_int(char* config){
  int count_elements = 0;
  for(unsigned int i=0; i<strlen(config); ++i){
    if(config[i] == ';') ++count_elements;
  }
  count_elements += 1;
  int* tree_classes = malloc(sizeof(int*)*count_elements);

  char* item = strtok(config, ";");
  int class_index = 0;
  while(item != NULL){
    tree_classes[class_index] = atoi(item);
    item = strtok(NULL, ";");
    ++class_index;
  }
  return tree_classes;
}

static int** parse_array_values(char* config, int *column_size){
  int count_classes = 0;
  for(unsigned int i=0; i<strlen(config); ++i){
    if(config[i] == ';') ++count_classes;
  }
  count_classes += 1;
  int firstpass = 1;

  int** values = malloc(sizeof(int*)*count_classes);
  char* item = strtok(config, ";");
  int index = 0;
  int size_2d = 0;
  while(item != NULL){
    // Count size of second dimension on the first pass
    if(firstpass){
      for(unsigned int i=0; i<strlen(item); ++i){
        if(item[i] == '.') ++size_2d;
      }
      firstpass = 0;
      *column_size = size_2d;
    }
    values[index] = malloc(sizeof(int*)*size_2d);
    char* saveptr;
    char* item_value;
    item_value = strtok_r(item, ".", &saveptr);
    for (int i = 0; i < size_2d; ++i) {
      values[index][i] = atoi(item_value);
      item_value = strtok_r(NULL, ".", &saveptr);
    }
    item = strtok(NULL, ";");
    ++index;
  }
  return values;
}


void scilC_algo_chooser_initialize() {
  int ret;

  /*
   * Handling of optional variable - compressor mapping file
   */
  if (getenv("SCIL_VARIABLE_MAPPING_FILE")) {
    char *var_compressor_file = getenv("SCIL_VARIABLE_MAPPING_FILE");
    FILE *var_compressor_data = fopen(var_compressor_file, "r");
    if (var_compressor_data == NULL) {
      critical("Could not open configuration file %s\n", var_compressor_file);
    }
    char *line = NULL;
    size_t len = 0;
    int lines = 0; // -1 to skip header
    while (getline(&line, &len, var_compressor_data) != -1) {
      ++lines;
    }
    rewind(var_compressor_data);

    variable_dict = scilU_dict_create(lines - 1);
    size_t linenumber = 0;
    char *delimiter = ",";
    while (getline(&line, &len, var_compressor_data) != -1) {
      if (linenumber > 0) { // Skip header
        char *variable_name = strtok(line, delimiter);
        char *compressor_name = strtok(NULL, delimiter);
        scilU_dict_put(variable_dict, variable_name, compressor_name);
        //printf("Var: %s | Comp: %s", variable_name, scilU_dict_get(variable_dict, variable_name)->value);
      }
      ++linenumber;
    }
    fclose(var_compressor_data);
    if (line != NULL) {
      free(line);
    }
  }

  /*
   * Handling of decision tree support
   */
  if (getenv("SCIL_DECISION_TREE_FILE")) {
    char *decision_tree_file = getenv("SCIL_DECISION_TREE_FILE");
    FILE *decision_tree_data = fopen(decision_tree_file, "r");
    if (decision_tree_data == NULL) {
      critical("Could not open decision tree file %s\n", decision_tree_file);
    }
    char* line = NULL;
    size_t len = 0;
    char** tree_class_names = NULL;
    int* tree_left = NULL;
    int* tree_right = NULL;
    double* tree_thresholds = NULL;
    int** tree_classes = NULL;
    int* tree_indices = NULL;
    int column_size = 0;
    while (getline(&line, &len, decision_tree_data) != -1) {
      if (line[0] == '#'){ // Type descriptor
        if(strstr(line, "classes") != NULL){
          getline(&line, &len, decision_tree_data); // Work on next line
          tree_class_names = parse_array_chars(line);
        }else if(strstr(line, "left") != NULL){
          getline(&line, &len, decision_tree_data);
          tree_left = parse_array_int(line);
        }else if(strstr(line, "right") != NULL){
          getline(&line, &len, decision_tree_data);
          tree_right = parse_array_int(line);
        }else if(strstr(line, "thresholds") != NULL){
          getline(&line, &len, decision_tree_data);
          tree_thresholds = parse_array_double(line);
        }else if(strstr(line, "indices") != NULL){
          getline(&line, &len, decision_tree_data);
          tree_indices = parse_array_int(line);
        }else if(strstr(line, "values")){
          getline(&line, &len, decision_tree_data);
          tree_classes = parse_array_values(line, &column_size);
        }
      }
    }
    fclose(decision_tree_data);
    decision_tree = scilU_tree_create(tree_left, tree_right, tree_thresholds, tree_indices, tree_classes, tree_class_names, column_size);
    //scilU_tree_remove(decision_tree);
  }

  /*
   * System characteristics
   */
  char *filename = getenv("SCIL_SYSTEM_CHARACTERISTICS_FILE");
  if (filename == NULL) {
    filename = SYSTEM_CONFIGURATION_FILE;
  }
  FILE *data = fopen("scil.conf", "r");
  if (data == NULL) {
    data = fopen(filename, "r");
    if (data == NULL) {
      warn("Could not open configuration file %s\n", filename);
      return;
    }
  }

  // File format is in CSV, see "dev/scil.conf" for an example->
  char *buff = malloc(1024);
  int config_list_capacity = 100;
  config_list = (config_file_entry_t *) malloc(sizeof(config_file_entry_t) * config_list_capacity); // up to 1k entries

  while (true) {
    size_t length = 1024;
    int64_t linelength;
    linelength = getline(&buff, &length, data);
    if (linelength == -1) {
      break;
    }
    if (buff[0] == '#' || strlen(buff) < 5) {
      // ignore comments
      continue;
    }
    if (buff[strlen(buff) - 1] == '\n') buff[strlen(buff) - 1] = 0;

    if (buff[0] == '!') {
      // hardware limit
      char *pos = strstr(buff, " ");
      if (pos == NULL) {
        warn("Invalid configuration line \"%s\"\n", buff);
        continue;
      }
      *pos = 0;
      ret = scilU_add_hardware_limit(&buff[1], &pos[1]);
      if (ret != SCIL_NO_ERR) {
        warn("Invalid configuration line \"%s\"\n", buff);
      }
      continue;
    }

    config_file_entry_t *e = &config_list[config_list_size];
    char name[100];
    char pattern_name[100];
    int tokens =
        sscanf(buff, "%f; %s %s %f; %f; %f", &e->randomness, pattern_name, name, &e->c_speed, &e->d_speed, &e->ratio);
    if (tokens != 6) {
      warn("Parsing configuration line \"%s\" returned an error after token %d\n", buff, tokens);
      continue;
    }
    name[strlen(name) - 1] = 0;
    ret = scilU_chain_create(&e->chain, name);
    if (ret != SCIL_NO_ERR) {
      warn("Parsing configuration line \"%s\"; could not parse compressor chain \"%s\"\n", buff, name);
      continue;
    }
    debug("Configuration line %.3f; %s; %.1f; %.1f; %.3f\n",
          (double) e->randomness,
          name,
          (double) e->c_speed,
          (double) e->d_speed,
          (double) e->ratio);

    config_list_size++;
    if (config_list_size >= config_list_capacity) {
      config_list_capacity *= 5;
      config_list = (config_file_entry_t *) realloc(config_list, config_list_capacity * sizeof(config_file_entry_t));
      debug("Configuration list increasing size to %d\n", config_list_capacity);
    }
  }
  config_list = (config_file_entry_t *) realloc(config_list, config_list_size * sizeof(config_file_entry_t));
  free(buff);
  fclose(data);

  debug("Configuration, parsed %d lines\n", config_list_size);

  parse_losless_list();
}

void scilC_algo_chooser_execute(const void *restrict source,
                                const scil_dims_t *dims,
                                scil_context_t *ctx) {
  scil_compression_chain_t *chain = &ctx->chain;
  int ret;

  // at the moment we only set the compression algorith once
  if (chain->total_size != 0) {
    return;
  }
  char *chainEnv = getenv("SCIL_FORCE_COMPRESSION_CHAIN");
  if (chainEnv != NULL) {
    if (strcmp(chainEnv, "lossless") == 0) {
      ctx->lossless_compression_needed = 1;
    } else {
      ret = scilU_chain_create(chain, chainEnv);
      if (ret != SCIL_NO_ERR) {
        critical("The environment variable SCIL_FORCE_COMPRESSION_CHAIN is invalid with \"%s\"\n", chainEnv);
      }
      return;
    }
  }
  const size_t count = scil_dims_get_count(dims);

  if (count < 10) {
    // always use memcopy for small data
    ret = scilU_chain_create(chain, "memcopy");
    return;
  }

  size_t out_size = 15000;
  byte buffer[15000];
  size_t in_size = 10000;
  if (count < in_size) {
    in_size = count;
  }

  float r = scilU_get_data_randomness(source, in_size, buffer, out_size);
  if (ctx->lossless_compression_needed) {
    // we can only select byte compressors compress because data must be accurate!
  }
  // TODO: pick the best algorithm for the settings given in ctx...

  if (r > 95) {
    ret = scilU_chain_create(chain, "memcopy");
  } else {
    ret = scilU_chain_create(chain, "lz4");
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
