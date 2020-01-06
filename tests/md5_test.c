#include <stdio.h>
#include <stdlib.h>

#include "../OTA.h"

int main(int argc, char* argv[]) {
    uint8_t command[] = {CHECK_MD5, 1, 1};
    uint8_t* response = command_handler(command);

    for(int i = 0; i < *(response + 1); i++) printf("%ld", *(response + 2 + i));
    putchar('\n');

    return 0;
}