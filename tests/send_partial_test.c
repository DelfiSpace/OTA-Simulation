#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../OTA.h"
#include "../error_handler.h"

void get_partials(uint8_t* partials, FILE* f, uint16_t num_blocks) {
    if(f == NULL) return;
    rewind(f);
    fseek(f, METADATA_SIZE - 1, SEEK_SET);

    uint8_t temp[BLOCK_SIZE];
    for(uint16_t i = 0; i < num_blocks; i++) {
        fread(temp, sizeof(uint8_t), BLOCK_SIZE, f);
        uint8_t val = 0;

    	for(int i = 0; i < BLOCK_SIZE; i++) {
    		val = CRC_TABLE[val ^ temp[i]];
    	}

        partials[i] = val;
    }

    rewind(f);
}

int main(int argc, char* argv[]) {
    uint16_t num_blocks = 0;
    uint8_t* partials;

    uint8_t command[256] = {0};
    command[COMMAND_DESTINATION] = 18;
    command[COMMAND_SIZE] = 5;
    command[COMMAND_STATE] = COMMAND_REQUEST;
    command[COMMAND_METHOD] = START_OTA;
    command[COMMAND_PARAMETER] = 1;
    command[COMMAND_PARAMETER_SIZE] = 1;

    //Start OTA
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

    //Send metadata
    command[COMMAND_METHOD] = SEND_METADATA;
    fread(&command[COMMAND_PARAMETER], sizeof(uint8_t), METADATA_SIZE - 1, update);
    command[COMMAND_PARAMETER_SIZE] = METADATA_SIZE - 1;

    num_blocks = command[COMMAND_PARAMETER + NUM_BLOCKS_OFFSET - 1];
    
    response = command_handler(command);
    if(response[COMMAND_STATE] != COMMAND_ERROR) {
        for(int i = 0; i < response[COMMAND_SIZE]; i++) printf("%02X ", response[i]);
        putchar('\n');
    } else print_error(response[COMMAND_PARAMETER]);
    
    partials = malloc(num_blocks);
    get_partials(partials, update, num_blocks);

    //Send partial crcs
    int i = 0;
    command[COMMAND_METHOD] = SEND_PARTIAL_CRCS;
    for(i = 0; i < num_blocks / BLOCK_SIZE; i++) {
        memcpy(&command[COMMAND_PARAMETER], partials + i * BLOCK_SIZE, BLOCK_SIZE);
        command[COMMAND_PARAMETER_SIZE] = BLOCK_SIZE;
        
        printf("Send partials(%d): ", BLOCK_SIZE);
        for(int i = 0; i < BLOCK_SIZE; i++) printf("%02X ", command[COMMAND_PARAMETER + i]);
        putchar('\n');

        response = command_handler(command);
        if(response[COMMAND_STATE] != COMMAND_ERROR) {
            for(int j = 0; j < response[COMMAND_SIZE]; j++) printf("%02X ", response[j]);
            putchar('\n');
        } else print_error(response[COMMAND_PARAMETER]);
    }

    if(num_blocks % BLOCK_SIZE > 0) {
        memcpy(&command[COMMAND_PARAMETER], partials + i * BLOCK_SIZE, num_blocks % BLOCK_SIZE);
        command[COMMAND_PARAMETER_SIZE] = num_blocks % BLOCK_SIZE;

        printf("Send partials(%d): ", num_blocks % BLOCK_SIZE);
        for(int i = 0; i < num_blocks % BLOCK_SIZE; i++) printf("%02X ", command[COMMAND_PARAMETER + i]);
        putchar('\n');

        response = command_handler(command);
        if(response[COMMAND_STATE] != COMMAND_ERROR) {
            for(int i = 0; i < response[COMMAND_SIZE]; i++) printf("%02X ", response[i]);
            putchar('\n');
        } else print_error(response[COMMAND_PARAMETER]);
    }

    //Stop OTA
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