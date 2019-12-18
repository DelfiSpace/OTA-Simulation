#include <stdio.h>
#include <stdlib.h>

#include "../OTA.h"
#include "../update_file.h"

int main(int argc, char* argv[]) {
    struct Slot* update = malloc(sizeof(struct Slot));;
    struct Slot* slot1 = malloc(sizeof(struct Slot));;

    initUpdate(update, "flash_file.bin");
    initSlot(slot1, "slot1.bin", 1);    

    print_metadata(update);

    FILE* update_file = fopen("slots/flash_file.bin", "r");

    if(update_file == NULL) {
        fprintf(stderr, "Can't open  file\n");
        return -1;
    }

    if(slot1->meta->status == FULL) {
        erase(slot1);
    }

    print_metadata(slot1);

    uint8_t* block = malloc(sizeof(uint8_t)*BLOCK_SIZE);
    uint8_t* par_crcs = fetch_partial_crcs(update);
    
    fseek(update_file, METADATA_SIZE+1, SEEK_SET);

    if(!start_update(slot1, update)) {
        printf("Error while starting update!\n");
        return -1;
    }
    printf("Start Updating...\n");

    for(int i = 0; i < update->meta->num_blocks; i++) {
        fread(block, sizeof(uint8_t), BLOCK_SIZE, update_file);
        if(!get_next_block(block)) printf("Block %d: Failed to send block!\n", i);
    }

    check_md5(slot1);
    stop_update();
    
    fclose(update_file);

    print_metadata(slot1);
    return 0;
}