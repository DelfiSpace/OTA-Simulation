#include <stdbool.h>
#include "def.h"

#ifndef OTA_H_
#define OTA_H_

void command_handler();

void start_OTA();
void receive_metadata();
void send_metadata();
void receive_partial_crcs();
void receive_block();
void check_partial_crc();
void check_md5();
void write_to_flash();
void stop_OTA();

#endif