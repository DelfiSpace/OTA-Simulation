#include <stdio.h>
#include <stdlib.h>

#include "../OTA.h"

int main(int argc, char* argv[]) {
    struct metadata* meta = malloc(sizeof(struct metadata));
    struct Slot* update = malloc(sizeof(struct Slot));
    struct Slot* slot0 = malloc(sizeof(struct Slot));


    initSlot(update, 0, "flash_file");

    get_slot_metadata(update, meta);

    print_metadata(update->descriptor, meta);

    FILE* update_file = fopen("slots/flash_file.bin", "r");

    if(update_file == NULL) {
        fprintf(stderr, "Can't open  file\n");
        return -1;
    }

    uint8_t* block = malloc(sizeof(uint8_t)*BLOCK_SIZE);
    fseek(update_file, METADATA_SIZE, SEEK_SET);

    for(int i = 0; i < meta->num_blocks; i++) {
        fread(block, sizeof(uint8_t), BLOCK_SIZE, update_file);
        printf("%s", block);
        if(!get_next_block(block)) printf("Error!\n");
    }
    fclose(update_file);
    return 0;
}