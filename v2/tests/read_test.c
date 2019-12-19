#include <stdio.h>
#include <stdlib.h>

#include "../slot_handler.h"

int main(int argc, char* argv[]) {
    struct Slot* slot1 = malloc(sizeof(struct Slot));

    initSlot(slot1, 1);
    print_metadata(slot1);

    return 0;
}