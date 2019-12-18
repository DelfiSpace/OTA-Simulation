#include <stdio.h>
#include <stdint.h>

#include "../def.h"

int main(int argc, char* argv[]) {
    FILE* file = fopen("slots/fram.bin", "w");
    if(file == NULL) {
        return -1;
    }

    for(int i = 0; i < FRAM_SIZE; i++) {
        putc(0, file);
    }

    fclose(file);
    return 0;
}