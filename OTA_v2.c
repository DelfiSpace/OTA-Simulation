#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <openssl/md5.h>

#include "OTA_v2.h"

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