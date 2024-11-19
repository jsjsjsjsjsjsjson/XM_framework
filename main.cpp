#include <stdio.h>
#include <stdint.h>
#include "CommandLineInterface.hpp"
#include "xm_file.hpp"
#include "audio_api.h"

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
    audio_handle_t *handle = NULL;
    audio_init_params_t params = {
        .sample_rate = 22050,
        .channels = 1,
        .format = AUDIO_FORMAT_S16,
        .buffer_size = 8192
    };

    // Initialize audio
    if (audio_initialize(&handle, &params) != AUDIO_SUCCESS) {
        fprintf(stderr, "Failed to initialize audio system\n");
        return 1;
    }
    xm_file.open_xm_file("madwreck_-_thunder.xm");
    printf("Open: %s\n", xm_file.current_file_name);
    xm_file.load_xm_file();
    xm_file.print_xm_info();
    for (uint16_t i = 0; i < xm_file.header.num_instrument; i++) {
        printf("INSTRUMENT #%d\n", i);
        for (uint16_t s = 0; s < xm_file.instrument[i].num_sample; s++) {
            printf("PLAY SAMPLE #%d (%d bytes)...\n", s, xm_file.instrument[i].sample[s].length);
            int16_t *tmp = new int16_t[xm_file.instrument[i].sample[s].length];
            if (xm_file.instrument[i].sample[s].type.sample_bit == SAMPLE_8BIT) {
                for (uint32_t k = 0; k < xm_file.instrument[i].sample[s].length; k++) {
                    tmp[k] = ((int16_t*)xm_file.instrument[i].sample[s].data)[k] << 8;
                }
            } else {
                memcpy(tmp, xm_file.instrument[i].sample[s].data, xm_file.instrument[i].sample[s].length);
            }
            audio_write(handle, tmp, xm_file.instrument[i].sample[s].length);
            delete[] tmp;
            getchar();
        }
        printf("\n");
    }
    cli.begin("XM");
    cli.addCommand("print_pat", print_pattern_cmd);
    while (true) {
        cli.update();
    }
    return 0;
}