#include <stdbool.h>

#include "error_handler.h"

#include "def.h"

#ifndef SLOT_HANDLER_H_
#define SLOT_HANDLER_H_

//bool set_boot_slot(struct Slot* slot, bool always);

enum error_codes fram_write_bytes(uint32_t address, uint8_t* data, uint16_t len);
enum error_codes fram_read_bytes(uint32_t address, uint8_t* data, uint16_t len);

enum error_codes slot_write_bytes(uint8_t slot, uint32_t address, uint8_t* data, uint16_t len);
enum error_codes slot_read_bytes(uint8_t slot, uint32_t address, uint8_t* data, uint16_t len);

// bool erase(struct Slot* slot);

// bool start_update(struct Slot* slot, struct Slot* update);
// bool stop_update();

// bool get_next_block(uint8_t* next_block);
// bool send_block(uint8_t* block);

// bool check_md5(struct Slot* slot);
// uint8_t get_block_crc(uint8_t* block);

#endif