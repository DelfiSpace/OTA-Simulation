#include <stdint.h>

#ifndef ERROR_HANDLER_H_
#define ERROR_HANDLER_H_

enum error_codes{
    NO_ERROR,
    NO_FRAM_ACCESS,
    NO_SLOT_ACCESS,
    SLOT_OUT_OF_RANGE,
    MEMORY_FULL,
    PARAMETER_OVERLOAD
};

uint8_t* error_handler(uint8_t* data, enum error_codes error);
void set_error(uint8_t* data, enum error_codes error);

#endif