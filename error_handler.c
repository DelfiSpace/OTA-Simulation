#include "error_handler.h"
#include "def.h"

uint8_t* throw_error(uint8_t* data, enum error_codes error) {
    *data = error;
    return data;
}

void set_error(uint8_t* data, enum error_codes error) {
    data[COMMAND_STATE] = COMMAND_ERROR;
    data[COMMAND_PARAMETER_SIZE] = 1;
    data[COMMAND_PARAMETER] = error;
    data[COMMAND_SIZE]++;
}

void print_error(enum error_codes error) {
    printf("Error(%d): ", error);
    switch (error)
    {
    case NO_FRAM_ACCESS:
        puts("No access to FRAM.");
        break;
    case NO_SLOT_ACCESS:
        puts("No access to requested slot.");
        break;
    case SLOT_OUT_OF_RANGE:
        puts("Requested slot is out of range.");
        break;
    case MEMORY_FULL:
        puts("The memory is full.");
        break;
    case PARAMETER_OVERLOAD:
        puts("To many parameters provided for function.");
        break;
    case UPDATE_NOT_STARTED:
        puts("The update has not started yet.");
        break;
    case UPDATE_ALREADY_STARTED:
        puts("The update has already been started.");
        break;
    default:
        break;
    }
}