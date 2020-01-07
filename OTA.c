#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "md5.h"

#include "OTA.h"
#include "slot_handler.h"
#include "error_handler.h"

void start_OTA(uint8_t slot_number);
void receive_metadata(uint8_t* metadata, uint8_t size);
uint8_t* send_metadata(uint8_t slot_number);
void receive_partial_crcs();
void receive_block();
void check_partial_crc();
uint8_t* check_md5(uint8_t slot_number);
void write_to_flash();
void stop_OTA();

/*  Description: Takes in commands and handles them accordingly.
    Simultates the transmission between ground station and the satellite.

    Command pattern: CMD - SIZE - DATA(32 bytes) */
uint8_t* command_handler(uint8_t* command) {
    uint8_t* response = malloc(MAX_COMMAND_SIZE);
    response[COMMAND_DESTINATION] = command[COMMAND_SOURCE];
    response[COMMAND_SOURCE] = command[COMMAND_DESTINATION];
    response[COMMAND_SIZE] = 5;

    uint8_t* data;

    switch (command[COMMAND_METHOD])
    {
    case START_OTA:
        if(command[COMMAND_PARAMETER_SIZE] == 1) start_OTA(command[COMMAND_PARAMETER] - 1);
        break;
    case SEND_METADATA:
        receive_metadata(&response[COMMAND_PARAMETER], response[COMMAND_PARAMETER_SIZE]);
        break;
    case RECEIVE_METADATA:
        if(command[COMMAND_PARAMETER_SIZE] == 1) {
            data = send_metadata(command[COMMAND_PARAMETER] - 1);
            memcpy(&response[COMMAND_METHOD], data + 1, data[2]);
            response[COMMAND_SIZE] += data[2];
        }
        break;
    case SEND_PARTIAL_CRCS:
        receive_partial_crcs();
        break;
    case SEND_BLOCK:
        receive_block();
        break;
    case CHECK_MD5:
        if(command[COMMAND_PARAMETER_SIZE] == 1) return check_md5(command[COMMAND_PARAMETER] - 1);
        break;
    case STOP_OTA:
        stop_OTA();
        break;
    default:
        break;
    }

    free(data);
    return response;
}

void start_OTA(uint8_t slot_number) {

}

void receive_metadata(uint8_t* metadata, uint8_t size) {

}

uint8_t* send_metadata(uint8_t slot_number) {
    int error;
    uint8_t* data = malloc(65);
    data[1] = RECEIVE_METADATA;
    data[2] = METADATA_SIZE + 2;

    if((error = fram_read_bytes((METADATA_SIZE + PAR_CRC_SIZE) * slot_number, &data[3], METADATA_SIZE)) != NO_ERROR) return error_handler(data, error);
    return data;
}

void receive_partial_crcs() {

}

void receive_block() {

}

void check_partial_crc() {

}

uint8_t* check_md5(uint8_t slot_number) {
    int error;
    uint8_t* data = malloc(65);
    data[1] = CHECK_MD5;
    data[2] = CRC_SIZE + 1 + 2;

    MD5_CTX md5_c;
    MD5_Init(&md5_c);

    uint16_t num_blocks;
    if((error = fram_read_bytes((METADATA_SIZE + PAR_CRC_SIZE) * slot_number + NUM_BLOCKS_OFFSET, (uint8_t*)&num_blocks, sizeof(uint16_t))) != NO_ERROR) return error_handler(data, error);

    uint8_t meta_crc[CRC_SIZE];
    if((error = fram_read_bytes((METADATA_SIZE + PAR_CRC_SIZE) * slot_number + CRC_OFFSET, meta_crc, CRC_SIZE)) != NO_ERROR) return error_handler(data, error);

    uint8_t* buffer = malloc(num_blocks * sizeof(uint8_t));
    if((error = slot_read_bytes(slot_number, 0, buffer, num_blocks)) != NO_ERROR) {
        free(buffer);
        return error_handler(data, error);
    }

    MD5_Update(&md5_c, buffer, num_blocks);
    free(buffer);

    MD5_Final(&data[4], &md5_c);

    if(memcmp(&data[4], meta_crc, CRC_SIZE) == 0) {
        data[3] = true;
    } else {
        data[3] = false;
    }
    
    return data;
}

void write_to_flash() {

}

void stop_OTA() {

}