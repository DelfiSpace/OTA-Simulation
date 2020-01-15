#include "error_handler.h"
#include "def.h"

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
    case PARAMETER_MISMATCH:
        puts("Invalid parameter size provided to function.");
        break;
    case UPDATE_NOT_STARTED:
        puts("The update has not started yet.");
        break;
    case UPDATE_ALREADY_STARTED:
        puts("The update has already been started.");
        break;
    case METADATA_ALREADY_RECEIVED:
        puts("The metadata has already been received.");
        break;
    case METADATA_NOT_RECEIVED:
        puts("The metadata has not been received yet.");
        break;
    case PARTIAL_ALREADY_RECEIVED:
        puts("The partial crcs have already been received.");
        break;
    case PARTIAL_NOT_RECEIVED:
        puts("The partial crcs have not been received yet.");
        break;
    case CRC_MISMATCH:
        puts("A partial crc mismatch has occurred.");
        break;
    case MD5_MISMATCH:
        puts("The md5 hash does no match.");
        break;
    case OFFSET_OUT_OF_RANGE:
        puts("The requested offset is out of range.");
        break;
    case SLOT_NOT_EMPTY:
        puts("The requested slot is not empty yet.");
        break;
    default:
        break;
    }
}