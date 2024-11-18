#include <stdio.h>
#include <stdint.h>
#include "CommandLineInterface.hpp"
#include "xm_file.hpp"

XMFile xm_file;

CommandLineInterface cli;

void print_pattern_cmd(int argc, const char* argv[]) {
    if (argc < 6) {
        printf("%s <pattern> <start channels> <end channels> <start rows> <end rows>\n", argv[0]);
        return;
    }
    xm_file.print_pattern(strtol(argv[1], NULL, 0), strtol(argv[2], NULL, 0), strtol(argv[3], NULL, 0), strtol(argv[4], NULL, 0), strtol(argv[5], NULL, 0));
}

void load_xm_file_cmd(int argc, const char* argv[]) {

}

int main() {
    xm_file.open_xm_file("madwreck_-_thunder.xm");
    printf("Open: %s\n", xm_file.current_file_name);
    xm_file.load_xm_file();
    xm_file.print_xm_info();
    cli.begin("XM");
    cli.addCommand("print_pat", print_pattern_cmd);
    while (true) {
        cli.update();
    }
    return 0;
}