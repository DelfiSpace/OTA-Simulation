#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <openssl/md5.h>

#include "OTA.h"

void start_OTA();
void receive_metadata(uint8_t* metadata, uint8_t size);
void send_metadata();
void receive_partial_crcs();
void receive_block();
void check_partial_crc();
void check_md5();
void write_to_flash();
void stop_OTA();

/*
    Command pattern: CMD - SIZE - DATA(32 bytes)
*/
void command_handler(uint8_t* command) {
    uint8_t cmd = *command;
    uint8_t size = *(command + 1);
    uint8_t* data = command + 2;

    switch (cmd)
    {
    case START_OTA:
        start_OTA();
        break;
    case RECEIVE_METADATA:
        receive_metadata(data, size);
        break;
    case SEND_METADATA:
        send_metadata();
        break;
    case RECEIVE_PARTIAL_CRCS:
        receive_partial_crcs();
        break;
    case RECEIVE_BLOCK:
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
}

void start_OTA() {

}

void receive_metadata() {

}

void send_metadata() {

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