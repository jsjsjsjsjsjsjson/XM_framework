#ifndef EXTRA_FUNC_H
#define EXTRA_FUNC_H

#include <stdint.h>

#define HAS_NOTE(mask)           (IS_ORIGINAL_MODE(mask) || (((mask) & 0x80) && ((mask) & 0x01)))
#define HAS_INSTRUMENT(mask)     (IS_ORIGINAL_MODE(mask) || (((mask) & 0x80) && ((mask) & 0x02)))
#define HAS_VOLUME(mask)         (IS_ORIGINAL_MODE(mask) || (((mask) & 0x80) && ((mask) & 0x04)))
#define HAS_EFFECT_TYPE(mask)    (IS_ORIGINAL_MODE(mask) || (((mask) & 0x80) && ((mask) & 0x08)))
#define HAS_EFFECT_PARAM(mask)   (IS_ORIGINAL_MODE(mask) || (((mask) & 0x80) && ((mask) & 0x10)))
#define IS_COMPRESSED_MODE(mask) ((mask) & 0x80)
#define IS_ORIGINAL_MODE(mask)   (!((mask) & 0x80))

void parse_vol_cmd(uint8_t vol_cmd, char *mnemonic, uint8_t *val) {
    if (!mnemonic || !val) return;
    // *mnemonic = '\0';
    // *val = 0;
    if (vol_cmd >= 0x10 && vol_cmd <= 0x4F) {
        *mnemonic = 'v';
        *val = vol_cmd - 0x10;
    } else if (vol_cmd >= 0x60 && vol_cmd <= 0x6F) {
        *mnemonic = 'd';
        *val = vol_cmd - 0x60;
    } else if (vol_cmd >= 0x70 && vol_cmd <= 0x7F) {
        *mnemonic = 'c';
        *val = vol_cmd - 0x70;
    } else if (vol_cmd >= 0x80 && vol_cmd <= 0x8F) {
        *mnemonic = 'b';
        *val = vol_cmd - 0x80;
    } else if (vol_cmd >= 0x90 && vol_cmd <= 0x9F) {
        *mnemonic = 'a';
        *val = vol_cmd - 0x90;
    } else if (vol_cmd >= 0xA0 && vol_cmd <= 0xAF) {
        *mnemonic = 'u';
        *val = vol_cmd - 0xA0;
    } else if (vol_cmd >= 0xB0 && vol_cmd <= 0xBF) {
        *mnemonic = 'h';
        *val = vol_cmd - 0xB0;
    } else if (vol_cmd >= 0xC0 && vol_cmd <= 0xCF) {
        *mnemonic = 'p';
        *val = vol_cmd - 0xC0;
    } else if (vol_cmd >= 0xD0 && vol_cmd <= 0xDF) {
        *mnemonic = 'l';
        *val = vol_cmd - 0xD0;
    } else if (vol_cmd >= 0xE0 && vol_cmd <= 0xEF) {
        *mnemonic = 'r';
        *val = vol_cmd - 0xE0;
    } else if (vol_cmd >= 0xF0 && vol_cmd <= 0xFF) {
        *mnemonic = 'g';
        *val = vol_cmd - 0xF0;
    } else {
        *mnemonic = '\0';
        *val = 0;
    }
}

const char *note_table[12] = {"C-", "C#", "D-", "D#", "E-", "F-", "F#", "G-", "G#", "A-", "A#", "B-"};

void xm_note_to_str(uint8_t note, char output[4]) {
    if (note) {
        note--;
        int8_t i = note % 12;
        output[0] = note_table[i][0];
        output[1] = note_table[i][1];
        output[2] = 49 + (note / 12);
        output[3] = '\0';
    } else {
        output[0] = '.';
        output[1] = '.';
        output[2] = '.';
        output[3] = '\0';
    }
}

#endif