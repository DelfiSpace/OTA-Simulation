#include <stdio.h>
#include <stdlib.h>

#include "../OTA.h"

int main(int argc, char* argv[]) {
    struct Slot* update = malloc(sizeof(struct Slot));;
    struct Slot* slot0 = malloc(sizeof(struct Slot));;

    initSlot(update, "flash_file.bin");
    initSlot(slot0, "slot0.bin");    

    print_metadata(update);

    FILE* update_file = fopen("slots/flash_file.bin", "r");

    if(update_file == NULL) {
        fprintf(stderr, "Can't open  file\n");
        return -1;
    }

    uint8_t* block = malloc(sizeof(uint8_t)*BLOCK_SIZE);
    fseek(update_file, METADATA_SIZE, SEEK_SET);

    if(!start_update(slot0, update)) printf("Error while starting update!");
    printf("Start Updating...\n");

    for(int i = 0; i < update->meta->num_blocks; i++) {
        fread(block, sizeof(uint8_t), BLOCK_SIZE, update_file);
        if(!get_next_block(block)) printf("Block %d: Failed to send block!\n", i);
    }
    stop_update();
    fclose(update_file);

    print_metadata(slot0);
    return 0;
}