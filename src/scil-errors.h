#ifndef SCIL_ERRORS_H
#define SCIL_ERRORS_H

enum scil_error_code {
    SCIL_NO_ERR = 0,
    SCIL_BUFFER_ERR,
    SCIL_MEMORY_ERR,
    SCIL_EINVAL,
    SCIL_UNKNOWN_ERR,
    SCIL_PRECISION_ERR
};

const char* scil_strerr(enum scil_error_code err);

#endif
