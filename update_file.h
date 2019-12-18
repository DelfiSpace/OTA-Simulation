#include <stdbool.h>

#include "def.h"

#ifndef GENERATE_H_
#define GENERATE_H_

int generate_flash_file(int version, int num_blocks);
void initUpdate(struct Slot* slot, char* file_name);
bool get_update_metadata(struct Slot* slot);

#endif