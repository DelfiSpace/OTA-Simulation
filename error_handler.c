#include "error_handler.h"

uint8_t* error_handler(uint8_t* data, enum error_codes error) {
    data[0] = 1;
    data[2] = 1;
    data[3] = error;
    return data;
}