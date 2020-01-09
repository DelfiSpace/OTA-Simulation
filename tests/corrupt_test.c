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

    printf("Starting OTA... ");
    //Start OTA
    uint8_t* response = command_handler(command);
    if(response[COMMAND_STATE] == COMMAND_ERROR) print_error(response[COMMAND_PARAMETER]);
    else puts("Successful\n");

    FILE* update = fopen("./slots/flash_file.bin", "r");
    if(update == NULL) {
        puts("Unable to open update file!");
        return -1;
    }

    //Send metadata
    command[COMMAND_METHOD] = SEND_METADATA;
    fread(&command[COMMAND_PARAMETER], sizeof(uint8_t), METADATA_SIZE - 1, update);
    command[COMMAND_PARAMETER_SIZE] = METADATA_SIZE - 1;

    num_blocks = command[COMMAND_PARAMETER + NUM_BLOCKS_OFFSET - 1] | (command[COMMAND_PARAMETER + NUM_BLOCKS_OFFSET] << 8);

    printf("Sending metadata... ");
    response = command_handler(command);
    if(response[COMMAND_STATE] == COMMAND_ERROR) print_error(response[COMMAND_PARAMETER]);
    else puts("Successful\n");
    printf("Number of blocks to be sent: %d\n\n", num_blocks);
    
    partials = malloc(num_blocks);
    get_partials(partials, update, num_blocks);

    //Send partial crcs
    int count = 0;
    command[COMMAND_METHOD] = SEND_PARTIAL_CRCS;
    for(count = 0; count < num_blocks / BLOCK_SIZE; count++) {
        memcpy(&command[COMMAND_PARAMETER], partials + count * BLOCK_SIZE, BLOCK_SIZE);
        command[COMMAND_PARAMETER_SIZE] = BLOCK_SIZE;
        
        printf("%d: Sending partials(%d): ", count, BLOCK_SIZE);
        for(int i = 0; i < BLOCK_SIZE; i++) printf("%02X ", command[COMMAND_PARAMETER + i]);

        response = command_handler(command);
        if(response[COMMAND_STATE] == COMMAND_ERROR) print_error(response[COMMAND_PARAMETER]);
        else puts("... Successful\n");
    }

    //Send remaining partials
    if(num_blocks % BLOCK_SIZE > 0) {
        memcpy(&command[COMMAND_PARAMETER], partials + count * BLOCK_SIZE, num_blocks % BLOCK_SIZE);
        command[COMMAND_PARAMETER_SIZE] = num_blocks % BLOCK_SIZE;

        printf("%d: Sending partials(%d): ", count, num_blocks % BLOCK_SIZE);
        for(int i = 0; i < num_blocks % BLOCK_SIZE; i++) printf("%02X ", command[COMMAND_PARAMETER + i]);

        response = command_handler(command);
        if(response[COMMAND_STATE] == COMMAND_ERROR) print_error(response[COMMAND_PARAMETER]);
        else puts("... Successful");
    }
    putchar('\n');

    free(partials);

    //Send data blocks
    rewind(update);
    fseek(update, METADATA_SIZE - 1, SEEK_SET);

    command[COMMAND_METHOD] = SEND_BLOCK;
    for(int i = 0; i < num_blocks; i++) {
        command[COMMAND_PARAMETER] = i;
        command[COMMAND_PARAMETER + 1] = i >> 8;
        fread(&command[COMMAND_PARAMETER + 2], sizeof(uint8_t), BLOCK_SIZE, update);
        command[COMMAND_PARAMETER_SIZE] = BLOCK_SIZE + 2;
        for(int i = 0; i < BLOCK_SIZE; i++) if(command[COMMAND_PARAMETER + 1 + i] > (0xFF - 0xFF * 0.01)) command[COMMAND_PARAMETER + 1 + i]++;
        
        printf("%d: Sending block: ", i);
        for(int i = 0; i < BLOCK_SIZE; i++) printf("%02X ", command[COMMAND_PARAMETER + i + 2]);

        response = command_handler(command);
        if(response[COMMAND_STATE] == COMMAND_ERROR) print_error(response[COMMAND_PARAMETER]);
        else puts("... Successful");
    }
    putchar('\n');


    //Stop OTA
    command[COMMAND_METHOD] = STOP_OTA;
    command[COMMAND_PARAMETER_SIZE] = 0;

    printf("Stopping OTA... ");
    response = command_handler(command);
    if(response[COMMAND_STATE] == COMMAND_ERROR) print_error(response[COMMAND_PARAMETER]);
    else puts("MD5 Correct\n");

    free(response);
    puts("Press enter to continue ...");
    getchar();
    return 0;
}