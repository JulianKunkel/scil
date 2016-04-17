#include <scil.h>

static const char* scil_error_messages[] = {
   "Error code 0: SUCCESS.",
   "Error code 1: Buffer overflow.",
   "Error code 2: Not enough memory.",
   "Error code 3: Invalid argument."
};

const char* scil_strerr(enum scil_error_code err){
    return & scil_error_messages[err];
}
