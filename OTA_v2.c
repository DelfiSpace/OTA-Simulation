#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <openssl/md5.h>

#include "OTA_v2.h"

void start_OTA();
void receive_metadata();
void send_metadata();
void receive_partial_crcs();
void receive_block();
void check_partial_crc();
void check_md5();
void write_to_flash();
void stop_OTA();

void command_handler(uint8_t command) {
    switch (command)
    {
    case 0x00:
        start_OTA();
        break;
    case 0x01:
        receive_metadata();
        break;
    case 0x02:
        send_metadata();
        break;
    case 0x03:
        receive_partial_crcs();
        break;
    case 0x04:
        receive_block();
        break;
    case 0x05:
        check_partial_crc();
        break;
    case 0x06:
        check_md5();
        break;
    case 0x07:
        write_to_flash();
        break;
    case 0x08:
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