#include <stdint.h>

#ifndef ERROR_HANDLER_H_
#define ERROR_HANDLER_H_

enum error_codes{
    NO_ERROR,
    NO_FRAM_ACCESS,
    NO_SLOT_ACCESS,
    SLOT_OUT_OF_RANGE
};

uint8_t* error_handler(uint8_t* data, enum error_codes error);

#endif