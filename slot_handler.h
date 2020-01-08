#include <stdbool.h>

#include "error_handler.h"

#include "def.h"

#ifndef SLOT_HANDLER_H_
#define SLOT_HANDLER_H_

enum error_codes fram_write_bytes(uint32_t address, uint8_t* data, uint16_t len);
enum error_codes fram_read_bytes(uint32_t address, uint8_t* data, uint16_t len);

enum error_codes slot_write_bytes(uint8_t slot, uint32_t address, uint8_t* data, uint16_t len);
enum error_codes slot_read_bytes(uint8_t slot, uint32_t address, uint8_t* data, uint16_t len);

// bool erase(struct Slot* slot);

#endif