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
    METADATA_NOT_RECEIVED,
    PARTIAL_ALREADY_RECEIVED,
    PARTIAL_NOT_RECEIVED,
    CRC_MISMATCH,
    MD5_MISMATCH,
    OFFSET_OUT_OF_RANGE,
    SLOT_NOT_EMPTY
};

//void throw_error(unsigned char error);
void print_error(enum error_codes error);

#endif