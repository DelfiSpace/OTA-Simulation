#include <stdio.h>
#include <stdlib.h>

#include "../OTA.h"

int main(int argc, char* argv[]) {
    struct Slot* slot0 = malloc(sizeof(struct Slot));

    initSlot(slot0, "slot0.bin");
    print_metadata(slot0);
    if(check_md5(slot0)) {
        printf("MD5 CRC matches with slot contents.\n");
    } else {
        printf("MD5 CRC doesn't match with slot contents.\n");
    }
    return 0;
}