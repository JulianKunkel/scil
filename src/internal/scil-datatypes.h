#ifndef SCIL_DATATYPES_H
#define SCIL_DATATYPES_H

#include <stdint.h>

typedef unsigned char byte;

enum SCIL_Datatype {
  SCIL_TYPE_FLOAT = 0,
  SCIL_TYPE_DOUBLE,
  SCIL_TYPE_INT8,
  SCIL_TYPE_INT16,
  SCIL_TYPE_INT32,
  SCIL_TYPE_INT64, //
  SCIL_TYPE_STRING,
  SCIL_TYPE_BINARY
};

#define SCIL_DATATYPE_NUMERIC_MAX SCIL_TYPE_INT64

typedef enum SCIL_Datatype SCIL_Datatype_t;

#endif // SCIL_DATATYPES_H
