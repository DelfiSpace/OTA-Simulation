#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <openssl/md5.h>

#include "OTA.h"

FILE* update_file;
struct Slot* current_slot;
struct Slot* updating_slot;
fpos_t* update_pointer;

void initSlot(struct Slot* slot, uint8_t slot_number, char* file_name) {
    char dir[100] = "./slots/";
    strcat(dir, file_name);
    strcat(dir, ".bin");

    slot->number = slot_number;
    slot->file = dir;
    slot->descriptor = file_name;
}

bool get_slot_metadata(struct Slot* slot, struct metadata* meta) {
    FILE* file = fopen(slot->file, "r+");
    if(file == NULL) return false;

    fread(&(meta->status), sizeof(uint8_t), 1, file);
    fread(meta->crc, sizeof(uint8_t), CRC_SIZE, file);
    fread(&(meta->version), sizeof(uint32_t), 1, file);
    fread(&(meta->num_blocks), sizeof(uint16_t), 1, file);

    return true;
}

void print_metadata(char* descriptor, struct metadata* meta) {
    printf("Metadata of %s:\n", descriptor);
    printf("\tSlot status: ");
    switch (meta->status)
    {
        case EMPTY:
            printf("emtpy\n");
            break;
        case PARTIAL:
            printf("partial\n");
            break;
        case FULL:
            printf("full\n");
            break;
        case TRANSMISSION:
            printf("transmission\n");
            break;
        default:
            break;
    }
    printf("\tVersion: %d\n", meta->version);
    printf("\tNumber of blocks: %d\n", meta->num_blocks);
    printf("\tMD5 CRC: ");
    for(int i = 0; i < CRC_SIZE; i++) {
            printf("%x", meta->crc[i]);
    }
    printf("\n");
}

bool set_boot_slot(struct Slot* slot, bool always) {
    if(slot == NULL) return false;

    current_slot = slot;
    
    return true;
}

bool erase(struct Slot* slot) {
    FILE* file = fopen(slot->file, "r+");
    if(file == NULL) return false;

    printf("Are you sure you want to erase this slot? (Y/N): ");
    char resp = toupper(getchar());

    if(resp == 'Y') {
        printf("Erasing %s\n\n", slot->descriptor);
        for(int i = 0; i <= METADATA_SIZE; i++) {
            putc(0, file);
        }
        return true;
    } else if(resp != 'N') {
        printf("Invalid response. Erase canceled\n\n");
    }
    return false;
}

bool start_update(struct Slot* slot, struct metadata* meta, const char* update) {
    if(slot == NULL) return false;
    if(meta->status == FULL) return false;

    FILE* file = fopen(slot->file, "r+");
    if(file == NULL) return false;

    updating_slot = slot;

    FILE* update_file = fopen(update, "r+");
    if(update_file != NULL) {
        putc(PARTIAL, file);
        fseek(update_file, 1, SEEK_SET);
        uint8_t buff[(METADATA_SIZE)];
        fread(buff, sizeof(uint8_t), METADATA_SIZE, update_file);
        fwrite(buff, sizeof(uint8_t), METADATA_SIZE, file);
        fgetpos(file, update_pointer); 
        fclose(update_file);
    }

    state = UPDATE;
    return true;
}

bool stop_update() {
    if(state != UPDATE) return false;
    state = IDLE;
    return true;
}

bool get_next_block(uint8_t* next_block) {
    if(state != UPDATE) return false;
    send_block(next_block);
    return true;
}

bool send_block(uint8_t* block) {
    if(state != UPDATE) return false;

    FILE* file = fopen(updating_slot->file, "r+");
    if(file == NULL) return false;

    fsetpos(file, update_pointer);
    fwrite(block, sizeof(uint8_t), BLOCK_SIZE, file);
    fgetpos(file, update_pointer);
    return true;
}

bool check_md5(struct Slot* slot, uint8_t* md5) {
    return true;
}

bool get_block_crc(uint16_t* crc, uint8_t* block) {
    return true;
}