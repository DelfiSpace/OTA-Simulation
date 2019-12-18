#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <time.h>

#include <openssl/md5.h>

#include "update_file.h"

bool isnumber(char* string) {
    int i = 0;
    while(string[i] != '\0') {
        if(!isdigit(string[i])) {
            return false;
        }
        i++;
    }
    return true;
}

int generate_flash_file(int version, int num_bytes) {
    struct Metadata* meta = malloc(sizeof(struct Metadata));
    srand((unsigned int) time(NULL));

    meta->version = version;

    meta->num_blocks = num_bytes / BLOCK_SIZE;
    uint32_t rest = num_bytes % BLOCK_SIZE;
    if(rest > 0) meta->num_blocks++;

    printf("Version: %x; Number of blocks: %d\n", meta->version, meta->num_blocks);

    MD5_CTX md5_c;
    uint8_t digest[CRC_SIZE];
    MD5_Init(&md5_c);

    FILE *file = fopen("slots/flash_file.bin", "w");

    fseek(file, CRC_SIZE+1, SEEK_SET);
    fwrite(&(meta->version), sizeof(uint32_t), 1, file);
    fwrite(&(meta->num_blocks), sizeof(uint16_t), 1, file);
    fseek(file, meta->num_blocks, SEEK_CUR);
    
    if (file != NULL) {
        int j = 0;
        int index = 0;
        uint8_t val = 0;
        uint8_t* par_crcs = malloc(sizeof(uint8_t) * meta->num_blocks);
        
        for(int i = 0; i < num_bytes; i++) {
            uint8_t temp = (uint8_t)rand();
            MD5_Update(&md5_c, &temp, 1);
            fwrite(&temp, sizeof(uint8_t), 1, file);

            if(j == BLOCK_SIZE) {
                *(par_crcs + index) = val;
                index++;
                j = 0;
                val = 0;
            }
            val = CRC_TABLE[val ^ temp];
            j++;
        }
        if(rest > 0) {
            for(int i = 0; i < (BLOCK_SIZE - rest); i++) {
                uint8_t temp = 0;
                MD5_Update(&md5_c, &temp, 1);
                fwrite(&temp, sizeof(uint8_t), 1, file);
                val = CRC_TABLE[val ^ temp];
            }
        }
    	*(par_crcs + index) = val;

        MD5_Final(digest, &md5_c);

        rewind(file);
        putc(TRANSMISSION, file);
        fwrite(digest, sizeof(uint8_t), CRC_SIZE, file);
        fseek(file, METADATA_SIZE - PAR_CRC_SIZE + 1, SEEK_SET);
        fwrite(par_crcs, sizeof(uint8_t), meta->num_blocks, file);
        fclose(file);
    }
    return 0;
}

void initUpdate(struct Slot* slot, char* file_name) {
    slot->file = malloc(sizeof(char)*(strlen(file_name) + 20));

    strcpy(slot->file, "./slots/");
    strcat(slot->file, file_name);

    slot->descriptor = file_name;

    slot->meta = malloc(sizeof(struct Metadata));
    get_update_metadata(slot);
}

bool get_update_metadata(struct Slot* slot) {
    const char func_name[] = "get_slot_metadata";

    FILE* file = fopen(slot->file, "r");
    if(file == NULL){
        printf("%s: Can't open fram file!\n", func_name);
        return false;
    }
    
    fread(&(slot->meta->status), sizeof(uint8_t), 1, file);
    fread(slot->meta->crc, sizeof(uint8_t), CRC_SIZE, file);
    fread(&(slot->meta->version), sizeof(uint32_t), 1, file);
    fread(&(slot->meta->num_blocks), sizeof(uint16_t), 1, file);

    fclose(file);

    return true;
}

uint8_t* fetch_partial_crcs(struct Slot* slot) {
    const char func_name[] = "fetch_partial_crcs";

    FILE* file = fopen(slot->file, "r");
    if(file == NULL){
        printf("%s: Can't open fram file!\n", func_name);
        return NULL;
    }

    uint8_t* par_crcs = malloc(sizeof(uint8_t) * slot->meta->num_blocks);
    fseek(file, METADATA_SIZE - PAR_CRC_SIZE, SEEK_SET);
    fread(par_crcs, sizeof(uint8_t), slot->meta->num_blocks, file);

    return par_crcs;
} 