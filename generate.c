#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <time.h>

#include <openssl/md5.h>

#include "generate.h"

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

int generate_flash_file(int version, int num_blocks) {
    struct metadata* meta = malloc(sizeof(struct metadata));
    srand((unsigned int) time(NULL));

    meta->version = version;

    meta->num_blocks = num_blocks;

    printf("Version: %d; Number of blocks: %d\n", meta->version, meta->num_blocks);

    MD5_CTX md5_c;
    uint8_t digest[CRC_SIZE];
    MD5_Init(&md5_c);

    FILE *file = fopen("bin/flash_file.bin", "wb");

    fseek(file, CRC_SIZE+1, SEEK_SET);
    fwrite(&(meta->version), sizeof(uint32_t), 1, file);
    fwrite(&(meta->num_blocks), sizeof(uint16_t), 1, file);
    
    if (file != NULL) {
        for(int i = 0; i < meta->num_blocks*BLOCK_SIZE; i++) {
            uint8_t temp = (uint8_t)rand();
            MD5_Update(&md5_c, &temp, 1);
            fwrite(&temp, sizeof(uint8_t), 1, file);
        }

        MD5_Final(digest, &md5_c);

        rewind(file);
        putc(TRANSMISSION, file);
        fwrite(digest, sizeof(uint8_t), CRC_SIZE, file);
        
        fclose(file);
    }
    return 0;
}