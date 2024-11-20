#ifndef EXTRA_FUNC_H
#define EXTRA_FUNC_H

#include <stdint.h>
#include <math.h>

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

size_t encode_dpcm_8bit(int8_t *pcm_data, int8_t *dpcm_data, size_t num_samples) {
    dpcm_data[0] = pcm_data[0];
    int16_t accumulated_error = 0;
    size_t error_count = 0;

    for (size_t i = 1; i < num_samples; ++i) {
        int16_t diff = pcm_data[i] - pcm_data[i - 1] + accumulated_error;

        if (diff > 127) {
            accumulated_error = diff - 127;
            diff = 127;
            error_count++;
        } else if (diff < -128) {
            accumulated_error = diff + 128;
            diff = -128;
            error_count++;
        } else {
            accumulated_error = 0;
        }

        dpcm_data[i] = (int8_t)diff;
    }
    return error_count;
}

size_t encode_dpcm_16bit(int16_t *pcm_data, int16_t *dpcm_data, size_t num_samples) {
    dpcm_data[0] = pcm_data[0];
    int32_t accumulated_error = 0;
    size_t error_count = 0;

    for (size_t i = 1; i < num_samples; ++i) {
        int32_t diff = pcm_data[i] - pcm_data[i - 1] + accumulated_error;

        if (diff > 32767) {
            accumulated_error = diff - 32767;
            diff = 32767;
            error_count++;
        } else if (diff < -32768) {
            accumulated_error = diff + 32768;
            diff = -32768;
            error_count++;
        } else {
            accumulated_error = 0;
        }

        dpcm_data[i] = (int16_t)diff;
    }
    return error_count;
}

void decode_dpcm_8bit(int8_t *dpcm_data, int8_t *pcm_data, size_t num_samples) {
    pcm_data[0] = dpcm_data[0];
    for (size_t i = 1; i < num_samples; ++i) {
        pcm_data[i] = pcm_data[i - 1] + dpcm_data[i];
    }
}

void decode_dpcm_16bit(int16_t *dpcm_data, int16_t *pcm_data, size_t num_samples) {
    pcm_data[0] = dpcm_data[0];
    for (size_t i = 1; i < num_samples; ++i) {
        pcm_data[i] = pcm_data[i - 1] + dpcm_data[i];
    }
}

const float C4_FREQ = 8363.0f;
const float SEMITONE_RATIO = powf(2.0f, 1.0f / 12.0f);

float calc_sample_rate(int8_t rel_tone, int8_t fine_tune) {
    const float FINETUNE_RATIO = powf(SEMITONE_RATIO, 1.0f / 128.0f);
    float semitone_adj = powf(SEMITONE_RATIO, rel_tone);
    float fine_adj = powf(FINETUNE_RATIO, fine_tune);
    return C4_FREQ * semitone_adj * fine_adj;
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

float noteToFrequency(float bassfreq, int note) {
    return bassfreq * powf(SEMITONE_RATIO, note - 37);;
}

size_t bpmToTicksize(uint16_t bpm, uint32_t smp_rate) {
    return roundf((2.5f * smp_rate) / bpm);
}

#endif