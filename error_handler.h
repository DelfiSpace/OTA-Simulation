#include <stdint.h>

#ifndef ERROR_HANDLER_H_
#define ERROR_HANDLER_H_

enum error_codes{
    NO_ERROR,
    NO_FRAM_ACCESS,
    NO_SLOT_ACCESS,
    SLOT_OUT_OF_RANGE,
    MEMORY_FULL,
    PARAMETER_MISMATCH,
    UPDATE_NOT_STARTED,
    UPDATE_ALREADY_STARTED,
    METADATA_ALREADY_RECEIVED,
    METADATA_NOT_RECEIVED
};

uint8_t* throw_error(uint8_t* data, enum error_codes error);
void set_error(uint8_t* data, enum error_codes error);
void print_error(enum error_codes error);

#endif