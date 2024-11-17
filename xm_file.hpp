#ifndef XM_FILE_HPP
#define XM_FILE_HPP

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

class XMFile {
public:

    FILE *xm_file;

    typedef struct {
        char ID_text[17] = {'E', 'x', 't', 'e', 'n', 'd', 'e', 'd', ' ', 'm', 'o', 'd', 'u', 'l', 'e', ':', ' '};
        char name[20] = "";
        uint8_t ID1A = 0x1A;
        char tracker_name[20] = "ESP32Tracker";
        uint8_t tracker_version[2] = {0, 1};
        uint32_t header_size = 0x00000114;
        uint16_t song_len = 1;
        uint16_t restart_pos = 0;
        uint16_t num_channel = 4;
        uint16_t num_pattern = 1;
        uint16_t num_instrument = 0;
        uint16_t freq_mode = 0xffff;
        uint16_t def_tempo = 4;
        uint16_t def_bpm = 125;
        uint8_t order_table[256];
    } xm_header_t;

    typedef struct {
        uint32_t pattern_header_length = 9;
        uint8_t packing_type = 0; // Almost always 0. Doesn't mean anything.
        uint16_t num_rows_pattern = 64;
        uint16_t packed_data_size = 64 * 4;
    } xm_pattern_header_t;

    xm_header_t header;

    void open_xm_file(const char *filename) {
        fopen(filename, "rb+");
    }

    void close_xm_file() {
        fclose(xm_file);
    }

    void read_xm_header() {
        fseek(xm_file, 0, SEEK_SET);
        fread(&header, 1, 80, xm_file);
        fread(header.order_table, 1, header.song_len, xm_file);
    }

    void read_xm_pattern_header() {
        fseek(xm_file, header.header_size + 60, SEEK_SET);
    }
};

#endif