#include <stdio.h>

#include "../update_file.h"

int main(int argc, char* argv[]) {
    generate_flash_file(0xfc61b2b, 20);
    return 0;
}