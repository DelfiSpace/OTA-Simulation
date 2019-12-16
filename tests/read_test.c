#include <stdio.h>
#include <stdlib.h>

#include "../OTA.h"

int main(int argc, char* argv[]) {
    struct metadata* meta = malloc(sizeof(struct metadata));
    struct Slot* slot0 = malloc(sizeof(struct Slot));

    initSlot(slot0, 0, "slot0");
    printf("%p\n", slot0->file);

    get_slot_metadata(slot0, meta);
    printf("%p\n", slot0->file);

    print_metadata(slot0->descriptor, meta);
    printf("%p\n", slot0->file);
    return 0;
}