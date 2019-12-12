#include <stdbool.h>
#include "def.h"

#ifndef OTA_H_
#define OTA_H_

void initSlot(struct Slot* slot, uint8_t slot_number, char* file_name);
bool set_boot_slot(struct Slot* slot, bool always);
bool get_slot_metadata(struct Slot* slot, struct metadata* meta);
void print_metadata(char* descriptor, struct metadata* meta);
bool erase(struct Slot* slot);

bool start_update(struct Slot* slot, struct metadata* meta, const char* update);
bool stop_update();

bool get_next_blocks(uint8_t* next_block);
bool send_block(uint8_t* block);

bool check_md5(struct Slot* slot, uint8_t* md5);
bool get_block_crc(uint16_t* crc, uint8_t* block);

#endif