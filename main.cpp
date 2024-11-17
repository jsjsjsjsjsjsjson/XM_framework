#include <stdio.h>
#include <stdint.h>
#include "xm_file.hpp"

int main() {
    FILE *file = fopen("test.xm", "rb");
    read_xm_header(file);
    fclose(file);
    return 0;
}