#include <stdio.h>
#include <stdlib.h>

#include "../OTA.h"

int main(int argc, char* argv[]) {
    struct Slot* slot0 = malloc(sizeof(struct Slot));

    initSlot(slot0, "slot0.bin");

    print_metadata(slot0);
    erase(slot0);

    get_slot_metadata(slot0);
    print_metadata(slot0);
    return 0;
}