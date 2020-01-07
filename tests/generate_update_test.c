#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#include "../md5.h"
#include "../def.h"

uint32_t version = 0xd03778b;
uint32_t num_bytes = 5;

int main(int argc, char* argv[]) {
    srand((unsigned int) time(NULL));

    uint16_t num_blocks = num_bytes / BLOCK_SIZE;
    uint8_t rest = num_bytes % BLOCK_SIZE;
    if(rest > 0) num_blocks++;

    printf("Version: %x; Number of blocks: %d\n", version, num_blocks);

    MD5_CTX md5_c;
    uint8_t digest[CRC_SIZE];
    MD5_Init(&md5_c);

    FILE *file = fopen("slots/flash_file.bin", "w");

    fseek(file, CRC_SIZE+1, SEEK_SET);
    fwrite(&version, sizeof(uint32_t), 1, file);
    fwrite(&num_blocks, sizeof(uint16_t), 1, file);
    fseek(file, num_blocks, SEEK_CUR);
    
    if (file != NULL) {
        int j = 0;
        int index = 0;
        uint8_t val = 0;
        uint8_t* par_crcs = malloc(sizeof(uint8_t) * num_blocks);
        
        for(int i = 0; i < num_bytes; i++) {
            uint8_t temp = (uint8_t)rand();
            MD5_Update(&md5_c, &temp, 1);
            putc(temp, file);
            printf("%02X ", temp);

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
        puts("\n");
        for(int i = 0; i < num_blocks; i++) printf("%02X", par_crcs[i]);
        putchar('\n');

        MD5_Final(digest, &md5_c);

        rewind(file);
        putc(TRANSMISSION, file);
        fwrite(digest, sizeof(uint8_t), CRC_SIZE, file);
        fseek(file, METADATA_SIZE, SEEK_SET);
        fwrite(par_crcs, sizeof(uint8_t), num_blocks, file);
        fclose(file);
    }
    return 0;
}