#include <stdio.h>
#include <stdlib.h>

#include "../OTA.h"

int main(int argc, char* argv[]) {
    uint8_t command[] = {CHECK_MD5, 1, 1};
    uint8_t* response = command_handler(command);
    
    for(int i = 0; i < *(response + 1) -1; i++) printf("%02X", *(response + 3 + i));
    putchar('\n');

    if(*(response+2)) {
        puts("true");
    } else {
        puts("false");
    }
    return 0;
}