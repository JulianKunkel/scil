#include <scil-internal.h>

const char* scil_error_messages[] = {
   "Error code 0: No error.",
   "Error code 1: Buffer overflow.",
   "Error code 2: Not enough memory.",
};

void scil_print_error(const uint8_t error_code){
    fprintf(stderr, "%s\n", scil_error_messages[error_code]);
}
