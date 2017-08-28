#ifndef SCIL_DATATYPES_H
#define SCIL_DATATYPES_H

#include <stdint.h>

typedef unsigned char byte;

enum SCIL_Datatype {
  SCIL_TYPE_UNKNOWN = 0,
  SCIL_TYPE_FLOAT,
  SCIL_TYPE_DOUBLE,
  SCIL_TYPE_INT8,
  SCIL_TYPE_INT16,
  SCIL_TYPE_INT32,
  SCIL_TYPE_INT64,
  SCIL_TYPE_BINARY,
  SCIL_TYPE_STRING
};

#define SCIL_DATATYPE_NUMERIC_MAX SCIL_TYPE_INT64
#define SCIL_DATATYPE_NUMERIC_MIN SCIL_TYPE_FLOAT

typedef enum SCIL_Datatype SCIL_Datatype_t;

typedef struct{
  union {
    uint8_t uint8;
    uint16_t uint16;
    uint32_t uint32;
    uint64_t uint64;
    float  flt32;
    double flt64;
  } u;
  SCIL_Datatype_t typ;
} scil_value_t;

// needed to cope with the macros
#define SCIL_VALUE_double(val) val.u.flt64
#define SCIL_VALUE_float(val) val.u.flt32
#define SCIL_VALUE_uint8_t(val) val.u.uint8
#define SCIL_VALUE_uint16_t(val) val.u.uint16
#define SCIL_VALUE_uint32_t(val) val.u.uint32
#define SCIL_VALUE_uint64_t(val) val.u.uint64
//

SCIL_Datatype_t scil_str_to_datatype(const char * str);
const char * scil_datatype_to_str(SCIL_Datatype_t typ);

#endif // SCIL_DATATYPES_H
