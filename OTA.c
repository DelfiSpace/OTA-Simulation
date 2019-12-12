#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <openssl/md5.h>

#include "OTA.h"

// int main(int argc, char* argv[]) { 
//     struct metadata* meta = malloc(sizeof(struct metadata));

//     struct Slot* slot0 = malloc(sizeof(struct Slot));
//     struct Slot* slot1 = malloc(sizeof(struct Slot));
//     struct Slot* slot2 = malloc(sizeof(struct Slot));

//     initSlot(slot0, 0, fopen("slot0.bin", "r+"), "slot 0");
//     initSlot(slot1, 1, fopen("slot1.bin", "r+"), "slot 1");
//     initSlot(slot2, 2, fopen("slot2.bin", "r+"), "slot 2");

//     if(slot0->file == NULL || slot1->file == NULL || slot2->file == NULL) {
//         fprintf(stderr, "Can't open a file\n");
//         return -1;
//     }

//     get_slot_metadata(slot0, meta);
    
//     //erase(slot0);

//     if(!start_update(slot0, meta, "flash_file.bin")) fprintf(stderr, "Error while starting update\n");

//     fclose(slot0->file); 
//     fclose(slot1->file); 
//     fclose(slot2->file); 
//     return 0;
// }

void initSlot(struct Slot* slot, uint8_t slot_number, char* file_name) {
    char dir[100] = "./slots/";
    strcat(dir, file_name);
    strcat(dir, ".bin");
    puts(dir);

    slot->number = slot_number;
    slot->file = fopen(dir, "r+");
    slot->descriptor = file_name;
}

bool get_slot_metadata(struct Slot* slot, struct metadata* meta) {
    fread(&(meta->status), sizeof(uint8_t), 1, slot->file);
    fread(meta->crc, sizeof(uint8_t), CRC_SIZE, slot->file);
    fread(&(meta->version), sizeof(uint32_t), 1, slot->file);
    fread(&(meta->num_blocks), sizeof(uint16_t), 1, slot->file);
    rewind(slot->file);

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
    printf("Are you sure you want to erase this slot? (Y/N): ");
    char resp = toupper(getchar());
    if(resp == 'Y') {
        printf("Erasing %s\n\n", slot->descriptor);
        for(int i = 0; i <= METADATA_SIZE; i++) {
            putc(0, slot->file);
        }
        rewind(slot->file);
        return true;
    } else if(resp != 'N') {
        printf("Invalid response. Erase canceled\n\n");
    }
    return false;
}

bool start_update(struct Slot* slot, struct metadata* meta, const char* update) {
    if(slot == NULL) return false;
    if(meta->status == FULL) return false;

    updating_slot = slot;

    FILE* update_file = fopen(update, "r+");
    if(update_file != NULL) {
        putc(FULL, slot->file);
        fseek(update_file, 1, SEEK_SET);
        uint8_t buff[(METADATA_SIZE)];
        fread(buff, sizeof(uint8_t), METADATA_SIZE, update_file);
        fwrite(buff, sizeof(uint8_t), METADATA_SIZE, slot->file);
        fclose(update_file);
    }

    state = UPDATE;
    return true;
}

void stop_update() {
    state = IDLE;
}

bool get_next_block(uint8_t* next_block) {
    return true;
}

bool send_block(uint8_t* block) {
    return true;
}

bool check_md5(struct Slot* slot, uint8_t* md5) {
    return true;
}

bool get_block_crc(uint16_t* crc, uint8_t* block) {
    return true;
}