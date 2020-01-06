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
    printf("\tVersion: %02x%02x%02x%02x\n", metadata[1], metadata[2], metadata[3], metadata[4]);
    printf("\tNumber of blocks: %d\n", metadata[6] << 8 | metadata[5]);
    printf("\tMD5 CRC: ");
    for(int i = 7; i < CRC_SIZE; i++) {
        printf("%02X", metadata[i]);
    }
    printf("\n");
}


int main(int argc, char* argv[]) {
    uint8_t command[] = {RECEIVE_METADATA, 1, 2};
    uint8_t* response = command_handler(command);

    printf("Data size: %d\n", *(response + 1));
    print_metadata(response + 2);

    return 0;
}