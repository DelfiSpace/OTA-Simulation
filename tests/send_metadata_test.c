#include <stdio.h>
#include <stdlib.h>

#include "../OTA.h"
#include "../error_handler.h"

int main(int argc, char* argv[]) {
    uint8_t command[256] = {0};
    command[COMMAND_DESTINATION] = 18;
    command[COMMAND_SIZE] = 5;
    command[COMMAND_STATE] = COMMAND_REQUEST;
    command[COMMAND_METHOD] = START_OTA;
    command[COMMAND_PARAMETER] = 1;
    command[COMMAND_PARAMETER_SIZE] = 1;

    uint8_t* response = command_handler(command);
    if(response[COMMAND_STATE] != COMMAND_ERROR) {
        for(int i = 0; i < response[COMMAND_SIZE]; i++) printf("%02X ", response[i]);
        putchar('\n');
    } else print_error(response[COMMAND_PARAMETER]);

    FILE* update = fopen("./slots/flash_file.bin", "r");
    if(update == NULL) {
        puts("Unable to open update file!");
        return -1;
    }

    command[COMMAND_METHOD] = SEND_METADATA;
    fread(&command[COMMAND_PARAMETER], sizeof(uint8_t), METADATA_SIZE - 1, update);
    command[COMMAND_PARAMETER_SIZE] = METADATA_SIZE - 1;
    
    response = command_handler(command);
    if(response[COMMAND_STATE] != COMMAND_ERROR) {
        for(int i = 0; i < response[COMMAND_SIZE]; i++) printf("%02X ", response[i]);
        putchar('\n');
    } else print_error(response[COMMAND_PARAMETER]);

    command[COMMAND_METHOD] = STOP_OTA;
    command[COMMAND_PARAMETER_SIZE] = 0;

    response = command_handler(command);
    if(response[COMMAND_STATE] != COMMAND_ERROR) {
        for(int i = 0; i < response[COMMAND_SIZE]; i++) printf("%02X ", response[i]);
        putchar('\n');
    } else print_error(response[COMMAND_PARAMETER]);

    free(response);
    return 0;
}