#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "slot_handler.h"

struct Slot* updating_slot;
fpos_t update_pointer;

enum error_codes fram_write_bytes(uint32_t address, uint8_t* data, uint16_t len) {
    const char* func_name = "fram_write_bytes";

    FILE* file = fopen(fram_file, "r+");
    if(file == NULL) return NO_FRAM_ACCESS;

    fseek(file, address, SEEK_SET);
    fwrite(data, sizeof(uint8_t), len, file);

    fclose(file);
    return NO_ERROR;
}

enum error_codes fram_read_bytes(uint32_t address, uint8_t* data, uint16_t len) {
    const char* func_name = "fram_read_bytes";

    FILE* file = fopen(fram_file, "r");
    if(file == NULL) return NO_FRAM_ACCESS;

    fseek(file, address, SEEK_SET);
    fread(data, sizeof(uint8_t), len, file);

    fclose(file);
    return NO_ERROR;
}

enum error_codes slot_write_bytes(uint8_t slot, uint32_t address, uint8_t* data, uint16_t len) {
    const char* func_name = "slot_write_bytes";

    FILE* file = fopen(slot_files[slot], "r+");
    if(file == NULL) return NO_SLOT_ACCESS;

    fseek(file, address, SEEK_SET);
    fwrite(data, sizeof(uint8_t), len, file);

    fclose(file);
    return NO_ERROR;
}

enum error_codes slot_read_bytes(uint8_t slot, uint32_t address, uint8_t* data, uint16_t len) {
    const char* func_name = "slot_read_bytes";

    FILE* file = fopen(slot_files[slot], "r");
    if(file == NULL) return NO_SLOT_ACCESS;

    fseek(file, address, SEEK_SET);
    fread(data, sizeof(uint8_t), len, file);

    fclose(file);
    return NO_ERROR;
}

// bool erase(struct Slot* slot) {
//     const char func_name[] = "erase";

//     FILE* file = fopen("slots/fram.bin", "r+");
//     if(file == NULL) return false;
//     if(slot->number < 1 || slot->number > 2) {
//         printf("%s: Slot is out of range shoud be > 0 or < 2! Slot number is %d.\n", func_name, slot->number);
//         return false;
//     }

//     fseek(file, METADATA_SIZE * (slot->number - 1), SEEK_SET);

//     printf("Are you sure you want to erase this slot? (Y/N): ");
//     char resp = toupper(getchar());

//     if(resp == 'Y') {
//         printf("Erasing %s\n\n", slot->descriptor);
//         for(int i = 0; i <= METADATA_SIZE - PAR_CRC_SIZE; i++) {
//             putc(0, file);
//         }
//         return true;
//     } else if(resp != 'N') {
//         printf("Invalid response. Erase canceled\n\n");
//     }

//     fclose(file);

//     get_slot_metadata(slot);
//     return false;
// }