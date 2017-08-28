#include <string.h>

#include <scil-datatypes.h>

static char * dtype_names[] = {
    "unknown",
    "float",
    "float16",
    "double",
    "int8",
    "int16",
    "int32",
    "int64",
    NULL
};

SCIL_Datatype_t scil_str_to_datatype(const char * str){
  int pos = 0;
  for(char ** cur = dtype_names; *cur != NULL; cur++){
    if (strcmp(*cur, str) == 0){
      return pos;
    }
    pos++;
  }
  return SCIL_TYPE_UNKNOWN;
}

const char * scil_datatype_to_str(SCIL_Datatype_t typ){
  return dtype_names[typ];
}
