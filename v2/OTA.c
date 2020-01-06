#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <openssl/md5.h>

#include "OTA.h"
#include "slot_handler.h"

void start_OTA(uint8_t slot_number);
void receive_metadata(uint8_t* metadata, uint8_t size);
uint8_t* send_metadata(uint8_t slot_number);
void receive_partial_crcs();
void receive_block();
void check_partial_crc();
void check_md5();
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
        if(size == 1) start_OTA(*data);
        break;
    case SEND_METADATA:
        receive_metadata(data, size);
        break;
    case RECEIVE_METADATA:
        if(size == 1) return send_metadata(*data);
        break;
    case SEND_PARTIAL_CRCS:
        receive_partial_crcs();
        break;
    case SEND_BLOCK:
        receive_block();
        break;
    case CHECK_MD5:
        check_md5();
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
    struct Metadata* meta = get_slot_metadata(slot_number);
    
    uint8_t* data = malloc(METADATA_SIZE + 2);
    *data = RECEIVE_METADATA;
    *(data + 1) = METADATA_SIZE;
    uint8_t* wr_pointer = data + 2;

    *wr_pointer = meta->status;
    wr_pointer += sizeof(uint8_t);

    memcpy(wr_pointer, &(meta->version), sizeof(uint32_t));
    wr_pointer += sizeof(uint32_t);

    memcpy(wr_pointer, &(meta->num_blocks), sizeof(uint16_t));
    wr_pointer += sizeof(uint16_t);

    memcpy(wr_pointer, meta->crc, CRC_SIZE);

    free(meta);
    return data;
}

void receive_partial_crcs() {

}

void receive_block() {

}

void check_partial_crc() {

}

void check_md5() {

}

void write_to_flash() {

}

void stop_OTA() {

}