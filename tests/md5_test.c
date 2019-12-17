#include <stdio.h>
#include <stdlib.h>

#include "../OTA.h"

int main(int argc, char* argv[]) {
    struct Slot* slot0 = malloc(sizeof(struct Slot));

    initSlot(slot0, "slot0.bin");
    print_metadata(slot0);
    check_md5(slot0);

    return 0;
}