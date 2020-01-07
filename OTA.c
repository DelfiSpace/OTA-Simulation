#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "md5.h"

#include "OTA.h"
#include "slot_handler.h"

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
    uint8_t cmd = *command;
    uint8_t size = *(command + 1);
    uint8_t* data = command + 2;

    switch (cmd)
    {
    case START_OTA:
        if(size == 1) start_OTA(*data - 1);
        break;
    case SEND_METADATA:
        receive_metadata(data, size);
        break;
    case RECEIVE_METADATA:
        if(size == 1) return send_metadata(*data - 1);
        break;
    case SEND_PARTIAL_CRCS:
        receive_partial_crcs();
        break;
    case SEND_BLOCK:
        receive_block();
        break;
    case CHECK_MD5:
        if(size == 1) return check_md5(*data - 1);
        break;
    case STOP_OTA:
        stop_OTA();
        break;
    default:
        break;
    }

    return NULL;
}

void start_OTA(uint8_t slot_number) {

}

void receive_metadata(uint8_t* metadata, uint8_t size) {

}

uint8_t* send_metadata(uint8_t slot_number) {
    uint8_t* data = malloc(METADATA_SIZE + 2);
    *data = RECEIVE_METADATA;
    *(data + 1) = METADATA_SIZE;

    fram_read_bytes((METADATA_SIZE + PAR_CRC_SIZE) * slot_number, data + 2, METADATA_SIZE);
    return data;
}

void receive_partial_crcs() {

}

void receive_block() {

}

void check_partial_crc() {

}

uint8_t* check_md5(uint8_t slot_number) {
    uint8_t* data = malloc(sizeof(uint16_t) + 2);
    *data = CHECK_MD5;
    *(data + 1) = CRC_SIZE + 1;

    MD5_CTX md5_c;
    MD5_Init(&md5_c);

    uint16_t num_blocks;
    fram_read_bytes((METADATA_SIZE + PAR_CRC_SIZE) * slot_number + NUM_BLOCKS_OFFSET, (uint8_t*)&num_blocks, sizeof(uint16_t));
    uint8_t meta_crc[CRC_SIZE];
    fram_read_bytes((METADATA_SIZE + PAR_CRC_SIZE) * slot_number + CRC_OFFSET, meta_crc, CRC_SIZE);

    uint8_t* buffer = malloc(num_blocks * sizeof(uint8_t));
    slot_read_bytes(slot_number, 0, buffer, num_blocks);
    MD5_Update(&md5_c, buffer, num_blocks);
    free(buffer);

    MD5_Final(data + 3, &md5_c);

    if(memcmp(data + 3, meta_crc, CRC_SIZE) == 0) {
        *(data + 2) = true;
    } else {
        *(data + 2) = false;
    }
    
    return data;
}

void write_to_flash() {

}

void stop_OTA() {

}