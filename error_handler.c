#include "error_handler.h"
#include "def.h"

uint8_t* error_handler(uint8_t* data, enum error_codes error) {
    *data = error;
    return data;
}

void set_error(uint8_t* data, enum error_codes error) {
    data[COMMAND_STATE] = COMMAND_ERROR;
    data[COMMAND_PARAMETER_SIZE] = 1;
    data[COMMAND_PARAMETER] = error;
    data[COMMAND_SIZE]++;
}