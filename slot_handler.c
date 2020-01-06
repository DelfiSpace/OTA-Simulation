#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "slot_handler.h"

struct Slot* updating_slot;
fpos_t update_pointer;

void fram_write_bytes(uint32_t address, uint8_t* data, uint16_t len) {
    const char* func_name = "write_bytes";

    FILE* file = fopen(fram_file, "r+");
    if(file == NULL) {
        printf("%s: Can't access FRAM!\n", func_name);
        return;
    }

    fseek(file, address, SEEK_SET);
    fwrite(data, sizeof(uint8_t), len, file);

    fclose(file);
}

void fram_read_bytes(uint32_t address, uint8_t* data, uint16_t len) {
    const char* func_name = "read_bytes";

    FILE* file = fopen(fram_file, "r");
    if(file == NULL) {
        printf("%s: Can't access FRAM!\n", func_name);
        return;
    }

    fseek(file, address, SEEK_SET);
    fread(data, sizeof(uint8_t), len, file);

    fclose(file);
}

// bool set_boot_slot(struct Slot* slot, bool always) {
//     if(slot == NULL) return false;

//     current_slot = slot;
    
//     return true;
// }

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

// bool start_update(struct Slot* slot, struct Slot* update) {
//     get_slot_metadata(slot);

//     const char func_name[] = "start_update";
//     if(slot == NULL) {
//         printf("%s: Slot not initialized!\n", func_name);
//         return false;
//     }
//     if(slot->meta->status == FULL) {
//         printf("%s: Slot is full, erase it first!\n", func_name);
//         return false;
//     }

//     FILE* file = fopen("slots/fram.bin", "r+");
//     if(file == NULL){
//         printf("%s: Can't open slot file!\n", func_name);
//         return false;
//     } 

//     if(slot->number < 1 || slot->number > 2) {
//         printf("%s: Slot is out of range shoud be > 0 or < 2! Slot number is %d.\n", func_name, slot->number);
//         return false;
//     }

//     fseek(file, METADATA_SIZE * (slot->number - 1), SEEK_SET);

//     updating_slot = slot;

//     FILE* update_file = fopen(update->file, "r");

//     if(update_file != NULL) {
//         slot->meta->status = PARTIAL;
//         putc(PARTIAL, file);

//         fseek(update_file, 1, SEEK_SET);

//         uint8_t buff[(METADATA_SIZE)];
//         fread(buff, sizeof(uint8_t), METADATA_SIZE, update_file);
//         fwrite(buff, sizeof(uint8_t), METADATA_SIZE, file);

//         fgetpos(file, &update_pointer); 

//         fclose(update_file);
//     }

//     fclose(file);
//     state = UPDATE;
//     return true;
// }

// bool stop_update() {
//     if(state != UPDATE) return false;
//     state = IDLE;
//     return true;
// }

// bool get_next_block(uint8_t* next_block) {
//     if(state != UPDATE) return false;
//     get_block_crc(next_block);
//     return send_block(next_block);
// }

// bool send_block(uint8_t* block) {
//     if(state != UPDATE) return false;

//     FILE* file = fopen(updating_slot->file, "r+");
//     if(file == NULL) return false;

//     fsetpos(file, &update_pointer);
//     fwrite(block, sizeof(uint8_t), BLOCK_SIZE, file);
//     fgetpos(file, &update_pointer);
    
//     fclose(file);
//     return true;
// }

// bool check_md5(struct Slot* slot) {
//     get_slot_metadata(slot);

//     FILE* file = fopen(slot->file, "r+");
//     if(file == NULL) return false;

//     uint8_t digest[CRC_SIZE];
//     MD5_CTX md5_c;
//     uint8_t* block = malloc(sizeof(uint8_t)*BLOCK_SIZE);

//     MD5_Init(&md5_c);
//     for(int i = 0; i < slot->meta->num_blocks; i++) {
//         fread(block, sizeof(uint8_t), BLOCK_SIZE, file);
//         MD5_Update(&md5_c, block, BLOCK_SIZE);
//     }

//     MD5_Final(digest, &md5_c);
//     if(memcmp(digest, slot->meta->crc, CRC_SIZE) != 0) {
//         printf("MD5 CRC doesn't match with slot contents.\n");
//         return false;
//     }

//     printf("MD5 CRC matches with slot contents.\n");
    
//     fclose(file);
//     putc(FULL, file);
//     fclose(file);
//     return true;
// }

// uint8_t get_block_crc(uint8_t* block) {
//     uint8_t val = 0;

// 	for(int i = 0; i < BLOCK_SIZE; i++) {
// 		val = CRC_TABLE[val ^ *(block + i)];
// 	}

//     return val;
// }