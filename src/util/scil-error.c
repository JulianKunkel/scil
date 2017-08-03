#include <scil-error.h>

static const char* scil_error_messages[] = {
    "Error code 0: SUCCESS.",
    "Error code 1: Buffer overflow.",
    "Error code 2: Not enough memory.",
    "Error code 3: Invalid argument.",
    "Error code 4: Unknown error.",
    "Error code 5: Precision too strict.",
    "Error code 6: Don't use fill value."
};

const char* scil_error_get_message(enum scil_error_code err)
{
    return scil_error_messages[err];
}
