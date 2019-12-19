#include <stdbool.h>

#include "def.h"

#ifndef SLOT_HANDLER_H_
#define SLOT_HANDLER_H_

void initSlot(struct Slot* slot, uint8_t slot_number);
//bool set_boot_slot(struct Slot* slot, bool always);
void get_slot_metadata(struct Slot* slot);
void print_metadata(struct Slot* slot);
// bool erase(struct Slot* slot);

// bool start_update(struct Slot* slot, struct Slot* update);
// bool stop_update();

// bool get_next_block(uint8_t* next_block);
// bool send_block(uint8_t* block);

// bool check_md5(struct Slot* slot);
// uint8_t get_block_crc(uint8_t* block);

#endif