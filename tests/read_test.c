#include <stdio.h>
#include <stdlib.h>

#include "../OTA.h"

void print_metadata(uint8_t* metadata) {
    printf("Metadata:\n");
    printf("\tSlot status: ");
    switch (metadata[0])
    {
        case EMPTY:
            printf("Emtpy\n");
            break;
        case PARTIAL:
            printf("Partial\n");
            break;
        case FULL:
            printf("Full\n");
            break;
        case TRANSMISSION:
            printf("Transmission\n");
            break;
        default:
            printf("Unknown slot status %d!\n", metadata[0]);
            break;
    }
    printf("\tVersion: %02x%02x%02x%02x\n", metadata[CRC_SIZE+1], metadata[CRC_SIZE+2], metadata[CRC_SIZE+3], metadata[CRC_SIZE+4]);
    printf("\tNumber of blocks: %d\n", metadata[CRC_SIZE+6] << 8 | metadata[CRC_SIZE+5]);
    printf("\tMD5 CRC: ");
    for(int i = 0; i < CRC_SIZE; i++) {
        printf("%02X", metadata[i + 1]);
    }
    printf("\n");
}


int main(int argc, char* argv[]) {
    uint8_t command[] = {RECEIVE_METADATA, 1, 1};
    uint8_t* response = command_handler(command);

    print_metadata(response + 2);

    return 0;
}