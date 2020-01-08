#include <stdio.h>
#include <stdlib.h>

#include "../OTA.h"

int main(int argc, char* argv[]) {
    uint8_t command[256] = {0};
    command[COMMAND_DESTINATION] = 18;
    command[COMMAND_SIZE] = 5;
    command[COMMAND_STATE] = COMMAND_REQUEST;
    command[COMMAND_METHOD] = CHECK_MD5;
    command[COMMAND_PARAMETER] = 1;
    command[COMMAND_PARAMETER_SIZE] = 1;

    uint8_t* response = command_handler(command);
    if(response[COMMAND_STATE] != COMMAND_ERROR) {
        printf("CRC correct: ");
        if(response[COMMAND_PARAMETER]) {
            puts("true");
        } else {
            puts("false");
        }
    } else {
        puts("Error occurred!");
        for(int i = 0; i < response[COMMAND_SIZE]; i++) printf("%02X ", response[i]);
        putchar('\n');
    }

    free(response);
    return 0;
}