#include <stdio.h>
#include <stdint.h>
#include "CommandLineInterface.hpp"
#include "xm_file.hpp"
#include "audio_api.h"

XMFile xm_file;
XMController xm_ctrl;
XMChannel xm_chl;

CommandLineInterface cli;

audio_handle_t *handle = NULL;
audio_init_params_t params = {
    .sample_rate = 22050,
    .channels = 1,
    .format = AUDIO_FORMAT_S16,
    .buffer_size = 4096
};

void print_pattern_cmd(int argc, const char* argv[]) {
    if (argc < 6) {
        printf("%s <pattern> <start channels> <end channels> <start rows> <end rows>\n", argv[0]);
        return;
    }
    xm_file.print_pattern(strtol(argv[1], NULL, 0), strtol(argv[2], NULL, 0), strtol(argv[3], NULL, 0), strtol(argv[4], NULL, 0), strtol(argv[5], NULL, 0));
}

void play_inst_cmd(int argc, const char* argv[]) {
    if (argc < 5) {
        printf("%s <inst num> <note> <tick> <tick_size>\n", argv[0]);
        return;
    }
    uint16_t note = strtol(argv[2], NULL, 0);
    uint16_t tick = strtol(argv[3], NULL, 0);
    size_t tick_size = strtol(argv[4], NULL, 0);
    xm_chl.init(&xm_ctrl);
    xm_chl.setNote(note);
    xm_chl.setInst(&xm_file.instrument[strtol(argv[1], NULL, 0)]);
    xm_chl.noteAttack();
    xm_chl.setVol(64);
    printf("TICK: %d, TICK_SIZE: %d Bytes, ", tick, tick_size * sizeof(audio16_t));
    audio16_t *abuf = new audio16_t[tick_size];
    printf("BUFFER IN %p\n", abuf);
    for (uint16_t i = 0; i < tick; i++) {
        xm_chl.processTick(abuf, tick_size);
        audio_write(handle, abuf, tick_size * sizeof(audio16_t));
        printf("ENV: %d\r", xm_chl.env_vol);
        if (i == tick / 5) {
            xm_chl.noteRelease();
            printf("RELEASE\n");
        }
    }
    printf("\n");
    delete[] abuf;
}

#include <unistd.h>

void play_pat_cmd(int argc, const char* argv[]) {
    if (argc < 3) {
        printf("%s <pat> <chl>\n", argv[0]);
        return;
    }
    uint16_t pat = strtol(argv[1], NULL, 0);
    uint16_t chl = strtol(argv[2], NULL, 0);
    xm_ctrl.init(&xm_file);
    audio16_t *abuf = new audio16_t[8192];
    size_t bufsize;
    while (1) {
        bufsize = xm_ctrl.processTick(abuf);
        printf("SIZE: %d Bytes\n", bufsize);
        audio_write(handle, abuf, bufsize);
        // usleep(5000);
        // getchar();
    }
}

int main() {
    // xm_file.open_xm_file("fod_nit.xm");
    xm_file.open_xm_file("Module1.xm");
    printf("Open: %s\n", xm_file.current_file_name);
    xm_file.load_xm_file();
    xm_file.print_xm_info();
    /*
    for (uint16_t i = 0; i < xm_file.header.num_instrument; i++) {
        printf("INSTRUMENT #%d\n", i);
        for (uint16_t s = 0; s < xm_file.instrument[i].num_sample; s++) {
            printf("PLAY SAMPLE #%d (%dHz %d bytes)...\n", s, xm_file.instrument[i].sample[s].smp_rate, xm_file.instrument[i].sample[s].length);
            int16_t *tmp = new int16_t[xm_file.instrument[i].sample[s].length];
            if (xm_file.instrument[i].sample[s].type.sample_bit == SAMPLE_8BIT) {
                for (uint32_t k = 0; k < xm_file.instrument[i].sample[s].length; k++) {
                    tmp[k] = ((int16_t*)xm_file.instrument[i].sample[s].data)[k] << 8;
                }
            } else {
                memcpy(tmp, xm_file.instrument[i].sample[s].data, xm_file.instrument[i].sample[s].length);
            }
            params.sample_rate = xm_file.instrument[i].sample[s].smp_rate;
            audio_set_params(handle, &params);
            audio_write(handle, tmp, xm_file.instrument[i].sample[s].length);
            delete[] tmp;
            getchar();
        }
        printf("\n");
    }
    */
    printf("\ninitialize audio system...\n");
    if (audio_initialize(&handle, &params) != AUDIO_SUCCESS) {
        fprintf(stderr, "Failed to initialize audio system\n");
        return 1;
    }
    cli.begin("XM");
    cli.addCommand("print_pat", print_pattern_cmd);
    cli.addCommand("play_inst", play_inst_cmd);
    cli.addCommand("play_pat", play_pat_cmd);
    params.sample_rate = SMP_RATE;
    audio_set_params(handle, &params);
    while (true) {
        cli.update();
    }
    return 0;
}