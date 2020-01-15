#include <stdio.h>
#include <stdlib.h>

#include "../OTA.h"
#include "../error_handler.h"

void print_response(uint8_t* response) {
    printf("Destination: %d\n", response[COMMAND_DESTINATION]);
    printf("Payload size: %d\n", response[COMMAND_SIZE]);
    printf("Source: %d\n", response[COMMAND_SOURCE]);
    printf("Service: %d\n", response[COMMAND_SERVICE]);
    printf("Response type: %d\n", response[COMMAND_RESPONSE]);
    printf("Method: %d\n", response[COMMAND_METHOD]);
    if(response[COMMAND_SIZE] > PAYLOAD_SIZE_OFFSET) {
        printf("Payload data: ");
        for(int i = 0; i < response[COMMAND_SIZE] - PAYLOAD_SIZE_OFFSET; i++) printf("%02X ", response[i + COMMAND_DATA]);
        puts("\n");
    } else {
        putchar('\n');
    }
}

int main(int argc, char* argv[]) {
    uint8_t command[256] = {0};
    command[COMMAND_DESTINATION] = 5;
    command[COMMAND_SIZE] = 1 + PAYLOAD_SIZE_OFFSET;
    command[COMMAND_SERVICE] = SOFTWAREUPDATE_SERVICE;
    command[COMMAND_RESPONSE] = COMMAND_REQUEST;
    command[COMMAND_METHOD] = START_OTA;
    command[COMMAND_DATA] = 2;

    uint8_t* response = command_handler(command);
    if(response[COMMAND_RESPONSE] != COMMAND_ERROR) {
        print_response(response);
    } else print_error(response[COMMAND_DATA]);

    response = command_handler(command);
    if(response[COMMAND_RESPONSE] != COMMAND_ERROR) {
        print_response(response);
    } else print_error(response[COMMAND_DATA]);

    command[COMMAND_METHOD] = STOP_OTA;
    command[COMMAND_SIZE] = PAYLOAD_SIZE_OFFSET;
    response = command_handler(command);
    if(response[COMMAND_RESPONSE] != COMMAND_ERROR) {
        print_response(response);
    } else print_error(response[COMMAND_DATA]);

    response = command_handler(command);
    if(response[COMMAND_RESPONSE] != COMMAND_ERROR) {
        print_response(response);
    } else print_error(response[COMMAND_DATA]);


    free(response);
    return 0;
}