#include <stdio.h>
#include <stdlib.h>

#include "../OTA.h"

int main(int argc, char* argv[]) {
    struct metadata* meta = malloc(sizeof(struct metadata));
    struct Slot* slot0 = malloc(sizeof(struct Slot));

    initSlot(slot0, 0, "slot0");

    if(slot0->file == NULL) {
        fprintf(stderr, "Can't open  file\n");
        return -1;
    }

    erase(slot0);

    get_slot_metadata(slot0, meta);

    print_metadata(slot0->descriptor, meta);
    return 0;
}