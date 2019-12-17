#include <stdio.h>

#include "../generate.h"

int main(int argc, char* argv[]) {
    generate_flash_file(0xfc61b2b, 200);
    return 0;
}