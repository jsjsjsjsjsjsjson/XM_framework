#ifndef XM_FILE_HPP
#define XM_FILE_HPP

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <vector>
#include <string>

#include "audio_struct.h"
#include "extra_func.h"
#include "EnvelopeProcessor.hpp"

#define SAFE_DELETE(ptr) do { \
    delete (ptr);             \
    (ptr) = nullptr;          \
} while(0)

#define NO_LOOP 0
#define FORWARD_LOOP 1
#define BIDI_LOOP 2

#define SAMPLE_8BIT 0
#define SAMPLE_16BIT 1

#define MAX_ROWS 256
#define MAX_CHANNELS 64

typedef struct {
    char ID_text[17] = {'E', 'x', 't', 'e', 'n', 'd', 'e', 'd', ' ', 'm', 'o', 'd', 'u', 'l', 'e', ':', ' '};
    char name[20] = "";
    uint8_t ID1A = 0x1A;
    char tracker_name[20] = "MicroTracker";
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
    std::vector<uint8_t> order_table;
} xm_header_t;

typedef struct {
    uint8_t note;
    uint8_t instrument;
    uint8_t vol_ctrl_byte;
    uint8_t effect_type;
    uint8_t effect_param;
    uint8_t mask;
} pattern_cell_t;

typedef struct __attribute__((packed)) {
    // ----PATTERN_HEADER----
    uint32_t header_length = 9;
    uint8_t packing_type = 0; // Almost always 0. Doesn't mean anything.
    uint16_t num_rows = 64;
    uint16_t packed_data_size = 64 * 4;
    // ----PATTERN_HEADER----

    // ---PATTERN_DATA----
    uint8_t *packed_data = NULL;
    pattern_cell_t **unpack_data = NULL;
    // ---PATTERN_DATA----
} xm_pattern_t;

typedef struct {
    uint8_t loop_mode : 2;
    uint8_t reserved : 2;
    bool sample_bit : 1;
} sample_type_t;

typedef struct __attribute__((packed)) {
    uint32_t length = 0;
    uint32_t loop_start = 0;
    uint32_t loop_length = 0;
    uint8_t volume = 64;
    int8_t finetune = 0;
    sample_type_t type;
    uint8_t pan = 124;
    int8_t rela_note_num = 0;
    uint8_t mode = 0;
    char name[22] = "SAMPLE";
    void *data = NULL;
    uint32_t smp_rate;
} xm_sample_t;

typedef struct __attribute__((packed)) {
    // ----METADATA----
    uint32_t size;
    char name[22];
    uint8_t type; // Almost always 0. Doesn't mean anything.
    uint16_t num_sample;
    // ----METADATA----

    // ----DATA---- (if num_sample is not zero)
    /*This field appears to be completely ignored by Fast Tracker 2 and there are modules in the wild that have completely broken values in this field.
    It's better to ignore it and assume the sample header size is always 40.*/
    uint32_t sample_header_size;
    uint8_t keymap[96];
    env_point_t vol_env[12];
    env_point_t pan_env[12];
    uint8_t num_vol_point;
    uint8_t num_pan_point;
    uint8_t vol_sus_point;
    uint8_t vol_loop_start_point;
    uint8_t vol_loop_end_point;
    uint8_t pan_sus_point;
    uint8_t pan_loop_start_point;
    uint8_t pan_loop_end_point;
    env_type_t vol_env_type;
    env_type_t pan_env_type;
    uint8_t vib_type;
    uint8_t vib_sweep;
    uint8_t vib_depth;
    uint8_t vib_rate;
    uint16_t vol_fadeout;
    char reserved[22];
    // ----DATA----
    std::vector<xm_sample_t> sample;
} xm_instrument_t;

bool unpack_pattern_data(uint8_t* packed_data, uint32_t packed_size, 
                         uint16_t num_rows, uint16_t num_channels, 
                         pattern_cell_t** output_data);

class XMFile {
public:
    FILE* xm_file;
    char* current_file_name;
    xm_header_t header;

    std::vector<xm_pattern_t> pattern;
    std::vector<xm_instrument_t> instrument;

    void open_xm_file(const char* filename);
    void close_xm_file();
    void read_xm_header();
    void read_xm_pattern();
    void read_xm_sample(xm_instrument_t* inst);
    void read_xm_instrument();
    void print_pattern(uint16_t num, int startChl, int endChl, int startRow, int endRow);
    void print_xm_info();
    void load_xm_file();
};

#endif