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
#include "envelope.hpp"

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

class XMFile;
class XMTrack;
class XMChannel;
class XMController;

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

bool unpack_pattern_data(uint8_t *packed_data, uint32_t packed_size, 
                         uint16_t num_rows, uint16_t num_channels, 
                         pattern_cell_t **output_data) {
    if (!packed_data || !output_data) {
        return false;
    }

    uint32_t data_index = 0;
    for (uint16_t row = 0; row < num_rows; ++row) {
        for (uint16_t channel = 0; channel < num_channels; ++channel) {
            if (data_index >= packed_size) {
                return false;
            }

            pattern_cell_t cell = {0};
            cell.mask = packed_data[data_index++];

            if (cell.mask & 0x80) {
                uint8_t mask_flags = cell.mask & 0x1F;

                if (mask_flags & 0x01) {
                    if (data_index >= packed_size) {
                        return false;
                    }
                    cell.note = packed_data[data_index++];
                } else {
                    cell.note = 0;
                }

                if (mask_flags & 0x02) {
                    if (data_index >= packed_size) {
                        return false;
                    }
                    cell.instrument = packed_data[data_index++];
                } else {
                    cell.instrument = 0;
                }

                if (mask_flags & 0x04) {
                    if (data_index >= packed_size) {
                        return false;
                    }
                    cell.vol_ctrl_byte = packed_data[data_index++];
                } else {
                    cell.vol_ctrl_byte = 0;
                }

                if (mask_flags & 0x08) {
                    if (data_index >= packed_size) {
                        return false;
                    }
                    cell.effect_type = packed_data[data_index++];
                } else {
                    cell.effect_type = 0;
                }

                if (mask_flags & 0x10) {
                    if (data_index >= packed_size) {
                        return false;
                    }
                    cell.effect_param = packed_data[data_index++];
                } else {
                    cell.effect_param = 0;
                }
            } else {

                if (data_index + 4 >= packed_size) {
                    return false;
                }

                cell.note = cell.mask;
                cell.instrument = packed_data[data_index++];
                cell.vol_ctrl_byte = packed_data[data_index++];
                cell.effect_type = packed_data[data_index++];
                cell.effect_param = packed_data[data_index++];
            }

            output_data[channel][row] = cell;
        }
    }

    return true;
}

class XMFile {
public:
    FILE *xm_file;

    char *current_file_name = NULL;

    xm_header_t header;
    std::vector<xm_pattern_t> pattern;
    std::vector<xm_instrument_t> instrument;

    void open_xm_file(const char *filename) {
        xm_file = fopen(filename, "rb+");
        current_file_name = (char*)realloc(current_file_name, strlen(filename));
        strcpy(current_file_name, filename);
    }

    void close_xm_file() {
        fclose(xm_file);
    }

    void read_xm_header() {
        fseek(xm_file, 0, SEEK_SET);
        fread(&header, 1, 80, xm_file);
        header.order_table.resize(header.song_len);
        fread(header.order_table.data(), 1, header.song_len, xm_file);
        for (uint8_t i = 0; i < header.song_len; i++) {
            printf("%d ", header.order_table[i]);
        }
        printf("\n");
    }

    void read_xm_pattern() {
        fseek(xm_file, header.header_size + 60, SEEK_SET);
        for (uint16_t p = 0; p < header.num_pattern; p++) {
            xm_pattern_t pat_tmp;
            fread(&pat_tmp, 1, 9, xm_file);
            printf("READ PATTERN #%d IN %p | SIZE: %d, CHAN: %d, ROWS: %d\n", p, ftell(xm_file), pat_tmp.packed_data_size, header.num_channel, pat_tmp.num_rows);
            pat_tmp.packed_data = new uint8_t[pat_tmp.packed_data_size];
            pat_tmp.unpack_data = new pattern_cell_t*[header.num_channel];
            for (uint16_t c = 0; c < header.num_channel; c++) {
                pat_tmp.unpack_data[c] = new pattern_cell_t[pat_tmp.num_rows];
                // printf("%p\n", pat_tmp.unpack_data[c]);
            }
            fread(pat_tmp.packed_data, 1, pat_tmp.packed_data_size, xm_file);
            printf("UNPACK: %p\n", (void*)pat_tmp.unpack_data);
            unpack_pattern_data(pat_tmp.packed_data, pat_tmp.packed_data_size, pat_tmp.num_rows, header.num_channel, pat_tmp.unpack_data);
            pattern.push_back(pat_tmp);
        }
    }

    void read_xm_sample(xm_instrument_t *inst) {
        for (uint16_t s = 0; s < inst->num_sample; s++) {
            xm_sample_t smp_tmp;
            printf("READ SAMPLE #%d METADATA IN %p\n", inst->sample.size(), ftell(xm_file));
            fread(&smp_tmp, 1, 40, xm_file);
            smp_tmp.smp_rate = roundf(calc_sample_rate(smp_tmp.rela_note_num, smp_tmp.finetune));
            inst->sample.push_back(smp_tmp);
            printf("SAMPLE %s\n%s, SIZE: %d bytes, FINETUNE: %d, RELATIVE NOTE: %d (SAMPLE_RATE: %dHz)\n\n", smp_tmp.name, smp_tmp.type.sample_bit == SAMPLE_8BIT ? "8BIT" : "16BIT", smp_tmp.length,
                                                                                                                smp_tmp.finetune, smp_tmp.rela_note_num, smp_tmp.smp_rate);
        }
        for (uint16_t s = 0; s < inst->num_sample; s++) {
            printf("READ SAMPLE #%d IN %p\nALLOC %d bytes MEMORY...\n", s, ftell(xm_file), inst->sample[s].length);
            inst->sample[s].data = new int8_t[inst->sample[s].length];
            printf("READING->%p...", inst->sample[s].data);
            fread(inst->sample[s].data, 1, inst->sample[s].length, xm_file);
            printf("READY.\n");
            printf("DECODE DPCM...");
            if (inst->sample[s].type.sample_bit == SAMPLE_8BIT) {
                decode_dpcm_8bit((int8_t*)inst->sample[s].data, (int8_t*)inst->sample[s].data, inst->sample[s].length);
            } else {
                decode_dpcm_16bit((int16_t*)inst->sample[s].data, (int16_t*)inst->sample[s].data, inst->sample[s].length / 2);
            }
            printf("SUCCESS\n\n");
        }
    }

    void read_xm_instrument() {
        for (uint16_t i = 0; i < header.num_instrument; i++) {
            xm_instrument_t inst_tmp;
            printf("READ INSTRUMENT #%d IN %p\n", i + 1, ftell(xm_file));
            fread(&inst_tmp, 1, 29, xm_file);
            printf("NAME: %s, SIZE: %d, NUM_SAMP: %d\n", inst_tmp.name, inst_tmp.size, inst_tmp.num_sample);
            if (inst_tmp.num_sample) {
                printf("READ METADATA AND ENVELOPE...\n");
                fseek(xm_file, -29, SEEK_CUR);
                fread(&inst_tmp, 1, 263, xm_file);
                printf("SAMPLE HEADER SIZE: %d\n", inst_tmp.sample_header_size);
                printf("SAMPLE KEYMAP:\n");
                for (uint8_t k = 0; k < 96; k++) {
                    printf("%d ", inst_tmp.keymap[k]);
                } printf("\n\n");

                printf("VOL ENV POINT (x: y)\n");
                if (inst_tmp.vol_env_type.on) {
                    for (uint8_t t = 0; t < inst_tmp.num_vol_point; t++) {
                        printf("%d: %d", inst_tmp.vol_env[t].x, inst_tmp.vol_env[t].y);
                        if (inst_tmp.vol_env_type.sus) {
                            if (t == inst_tmp.vol_sus_point) printf(" <- SUS");
                        }
                        if (inst_tmp.vol_env_type.loop) {
                            if (t == inst_tmp.vol_loop_start_point) printf(" <- LOOP START");
                            if (t == inst_tmp.vol_loop_end_point) printf(" <- LOOP END");
                        }
                        printf("\n");
                    } printf("\n\n");
                } else {
                    printf("VOL ENV IS DISABLE\n\n");
                }

                printf("PAN ENV POINT (x: y)\n");
                if (inst_tmp.pan_env_type.on) {
                    for (uint8_t t = 0; t < inst_tmp.num_pan_point; t++) {
                        printf("%d: %d", inst_tmp.pan_env[t].x, inst_tmp.pan_env[t].y);
                        if (inst_tmp.pan_env_type.sus) {
                            if (t == inst_tmp.pan_sus_point) printf(" <- SUS");
                        }
                        if (inst_tmp.pan_env_type.loop) {
                            if (t == inst_tmp.pan_loop_start_point) printf(" <- LOOP START");
                            if (t == inst_tmp.pan_loop_end_point) printf(" <- LOOP END");
                        }
                        printf("\n");
                    } printf("\n\n");
                } else {
                    printf("PAN ENV IS DISABLE\n\n");
                }
                read_xm_sample(&inst_tmp);
            } else {
                printf("NO SAMPLE, SKIP.\n");
            }
            instrument.push_back(inst_tmp);
        }
    }

    void print_pattern(uint16_t num, int startChl, int endChl, int startRow, int endRow) {
        printf("PATTERN #%d: Channel %d ~ %d, Row %d ~ %d\n", num, startChl, endChl, startRow, endRow);
        printf("┌────");
        for (int i = startChl; i < endChl; i++) {
            printf("─────────────────────");
        }
        printf("┐\n");

        printf("│    ");
        for (int i = startChl; i < endChl; i++) {
            printf("┌────┬──────────────┐");
        }
        printf("│\n│    ");
        for (int i = startChl; i < endChl; i++) {
            printf("│Mask│  Channel %02d  │", i);
        }
        printf("│\n├────");
        for (int i = startChl; i < endChl; i++) {
            printf("┼────┼──────────────┤");
        }
        printf("│\n");

        for (int r = startRow; r < endRow; r++) {
            printf("│ %02X ", r);
            for (int c = startChl; c < endChl; c++) {
                pattern_cell_t tmp = pattern[num].unpack_data[c][r];
                printf("│0x%02X│", tmp.mask);
                if (HAS_NOTE(tmp.mask)) {
                    char note_tmp[4];
                    xm_note_to_str(tmp.note, note_tmp);
                    // printf("%s ", note_tmp);
                    printf("%03d ", tmp.note);
                } else {
                    printf("... ");
                }

                if (HAS_INSTRUMENT(tmp.mask))
                    printf("%02d ", tmp.instrument);
                else
                    printf(".. ");

                if (HAS_VOLUME(tmp.mask)) {
                    char vol_cmd;
                    uint8_t vol_cmd_val;
                    parse_vol_cmd(tmp.vol_ctrl_byte, &vol_cmd, &vol_cmd_val);
                    printf("%c%02d ", vol_cmd, vol_cmd_val);
                } else {
                    printf("... ");
                }

                if (HAS_EFFECT_TYPE(tmp.mask)) {
                    if (tmp.effect_type > 0xF) {
                        printf("%c", tmp.effect_type + 55);
                    } else {
                        printf("%X", tmp.effect_type);
                    }
                } else {
                    printf(".");
                }

                if (HAS_EFFECT_PARAM(tmp.mask))
                    printf("%02X│", tmp.effect_param);
                else
                    printf("..│");
            }
            printf("│\n");
        }

        printf("└────");
        for (int i = startChl; i < endChl; i++) {
            printf("┴────┴──────────────┘");
        }
        printf("┘\n");
    }
    
    void print_xm_info() {
        printf("XM FILE INFO (HEADER=%.17s ID=0x%02X)\n", header.ID_text, header.ID1A);
        printf("FILENAME: %s, SONGNAME: %.20s\n", current_file_name, header.name);
        printf("CHANNELS: %d, PATTERNS: %d, INSTRUMENT: %d, SONGLEN: %d, RESTART IN %d\n", header.num_channel, header.num_pattern, header.num_instrument, header.song_len, header.restart_pos);
        printf("CREATED IN %.20s, VERSION %d.%d\n", header.tracker_name, header.tracker_version[1], header.tracker_version[0]);
        printf("INIT BPM=%d, TEMPO=%d\n", header.def_bpm, header.def_tempo);
        printf("ORDER TABLE:\n");
        for (uint16_t i = 0; i < header.song_len; i++) {
            printf("%d ", header.order_table[i]);
        }
        printf("\n\n");
    }

    void load_xm_file() {
        read_xm_header();
        read_xm_pattern();
        read_xm_instrument();
    }
};

typedef enum {
    SAMPLE_STOP,
    SAMPLE_PLAYING
} samp_state_t;

class XMChannel {
public:
    void init(XMController* controllerRef) {
        controller = controllerRef;
    }

    EnvelopeProcessor vol_envProc;
    EnvelopeProcessor pan_envProc;

    uint32_t freq = 0;
    uint8_t note = 0;
    uint16_t vol = 64;
    float sample_frac_index = 0;
    uint16_t env_vol = vol_envProc.getValue();
    uint32_t sample_int_index = 0;
    float increment = 0;

    samp_state_t samp_state = SAMPLE_STOP;

    xm_instrument_t *cur_inst;
    xm_sample_t *cur_sample;

    void setFreq(uint32_t freqRef) {
        freq = freqRef;
        increment = (float)freq / SMP_RATE;
        printf("SET FREQ: %d / %d = %f\n", freq, SMP_RATE, increment);
    }

    uint32_t getFreq() {
        return freq;
    }

    void setNote(uint8_t noteRef) {
        note = noteRef;
    }

    uint8_t getNote() {
        return note;
    }

    void setVol(uint16_t volRef) {
        vol = volRef;
    }

    uint16_t getVol() {
        return vol;
    }

    void setInst(xm_instrument_t *inst) {
        cur_inst = inst;
        vol_envProc.setEnvelope(cur_inst->vol_env, cur_inst->num_vol_point, cur_inst->vol_sus_point,
                                    cur_inst->vol_loop_start_point, cur_inst->vol_loop_end_point,
                                        cur_inst->vol_env_type, cur_inst->vol_fadeout);
        printf("SET INST(ENV): POINT = %d, SUS = %d, LOOP = %d ~ %d, TYPEMASK = 0x%X, FADEOUT = %d\n", cur_inst->num_vol_point, cur_inst->vol_sus_point,
                                                                                                        cur_inst->vol_loop_start_point, cur_inst->vol_loop_end_point,
                                                                                                            cur_inst->vol_env_type, cur_inst->vol_fadeout);
        cur_sample = &cur_inst->sample[cur_inst->keymap[note]];
        printf("SET SAMPLE: %d, SAMPLE #%d, VOLUME = %d\n", note, cur_inst->keymap[note], cur_sample->volume);
        setFreq(noteToFrequency(cur_sample->smp_rate, note));
    }

    xm_instrument_t *getInst() {
        return cur_inst;
    }

    xm_sample_t *getCurrentSample() {
        return cur_sample;
    }

    void noteAttack() {
        sample_int_index = 0;
        sample_frac_index = 0;
        samp_state = SAMPLE_PLAYING;
        vol = cur_sample->volume;
        vol_envProc.start();
    }

    void noteRelease() {
        vol_envProc.release();
    }

    size_t processSample(audio16_t *buf, size_t tick_size) {
        size_t sample_tick = 0;
        audio16_t result;
        for (size_t i = 0; i < tick_size; i++) {
            if (samp_state == SAMPLE_STOP || !vol) {
                result.l = 0, result.r = 0;
                buf[i].l = result.l, buf[i].r = result.r;
                continue;
            }
            if (cur_sample->type.sample_bit == SAMPLE_16BIT) {
                if (cur_sample->type.loop_mode) {
                    if (sample_int_index >= ((cur_sample->loop_start + cur_sample->loop_length) / 2)) {
                        sample_int_index -= cur_sample->loop_length / 2;
                    }
                } else if (sample_int_index >= cur_sample->length / 2) {
                    samp_state = SAMPLE_STOP;
                    buf[i].l = result.l, buf[i].r = result.r;
                    continue;
                }
                result.l = ((int16_t*)cur_sample->data)[sample_int_index];
                result.r = ((int16_t*)cur_sample->data)[sample_int_index];
            } else {
                if (cur_sample->type.loop_mode) {
                    if (sample_int_index >= (cur_sample->loop_start + cur_sample->loop_length)) {
                        sample_int_index -= cur_sample->loop_length;
                    }
                } else if (sample_int_index >= cur_sample->length) {
                    samp_state = SAMPLE_STOP;
                    buf[i].l = result.l, buf[i].r = result.r;
                    continue;
                }
                result.l = ((int8_t*)cur_sample->data)[sample_int_index] << 8;
                result.r = ((int8_t*)cur_sample->data)[sample_int_index] << 8;
            }
            uint32_t final_vol = env_vol * vol;
            result.l = (result.l * final_vol) >> 12;
            result.r = (result.r * final_vol) >> 12;
            sample_frac_index += increment;
            if (sample_frac_index >= 1.0f) {
                sample_int_index += (int)sample_frac_index;
                sample_frac_index -= (int)sample_frac_index;
            }
            buf[i].l = result.l, buf[i].r = result.r;
            sample_tick++;
        }
        return sample_tick;
    }

    size_t processTick(audio16_t *buf, size_t tick_size) {
        vol_envProc.next();
        env_vol = vol_envProc.getValue();
        return processSample(buf, tick_size);
    }

private:
    XMController* controller;
};

class XMTrack {
public:
    void init(XMFile *xmfileRef, XMController *controllerRef, XMChannel *channelRef) {
        xmfile = xmfileRef;
        controller = controllerRef;
        channel = channelRef;
    }

    void processEffect(uint8_t type, uint8_t param) {
        if (type == 0xF) {
            if (param < 32) {
                controller->setTempo(param);
            } else {
                controller->setSpeed(param);
            }
        }
    }

    void processRows(pattern_cell_t *cell) {
        if (HAS_NOTE(cell->mask)) {
            channel->setNote(cell->note);
            if (cell->note == 97) {
                channel->noteRelease();
            }
        }
        if (HAS_INSTRUMENT(cell->mask)) {
            channel->setInst(&xmfile->instrument[cell->instrument - 1]);
            if (channel->note == 97) {
                channel->noteRelease();
            } else {
                channel->noteAttack();
            }
        }
        if (HAS_VOLUME(cell->mask)) {
            char cmd;
            uint8_t val;
            parse_vol_cmd(cell->vol_ctrl_byte, &cmd, &val);
            if (cmd == 'v') {
                channel->setVol(val);
            }
        }
        if (HAS_EFFECT_TYPE(cell->mask)) {
            processEffect(cell->effect_type, cell->effect_param);
        }
    }

private:
    XMFile *xmfile;
    XMController *controller;
    XMChannel *channel;
};

class XMController {
public:
    size_t tick_size = 0;

    std::vector<XMChannel> xm_channel;
    std::vector<XMTrack> xm_track;
    uint16_t speed;
    uint16_t tempo;
    uint16_t tick_pos;

    void init(XMFile *xmfileRef) {
        xmfile = xmfileRef;
        printf("BPM: %d\n", xmfile->header.def_bpm);
        tick_size = bpmToTicksize(xmfile->header.def_bpm, SMP_RATE);

        xm_channel.resize(xmfile->header.num_channel);
        xm_track.resize(xmfile->header.num_channel);
        for (uint16_t i = 0; i < xmfile->header.num_channel; i++) {
            xm_channel[i].init(this);
            xm_track[i].init(xmfile, this, &xm_channel[i]);
        }

        speed = 151;//xmfile->header.def_bpm;
        tempo = 2;//xmfile->header.def_tempo;
        tick_pos = tempo;
    }

    uint16_t row_pos = 0;
    uint16_t order_pos = 11;
    uint16_t chl = 9;

    void setTempo(uint16_t tempoRef) {
        tempo = tempoRef;
        tick_pos = tempo;
    }

    uint16_t getTempo() {
        return tempo;
    }

    void setSpeed(uint16_t speedRef) {
        speed = speedRef;
        tick_size = bpmToTicksize(xmfile->header.def_bpm, SMP_RATE);
    }

    uint16_t getSpeed() {
        return speed;
    }

    size_t getTickSize() {
        return tick_size;
    }

    size_t processTick(audio16_t *obuf) {
        if (tick_pos >= tempo) {
            pattern_cell_t *cell = &xmfile->pattern[xmfile->header.order_table[order_pos]].unpack_data[chl][row_pos];
            printf("%02d: %03d %02d\n", row_pos, cell->note, cell->instrument);
            xm_track[0].processRows(cell);
            tick_pos = 0;
            row_pos++;
            if (row_pos >= xmfile->pattern[xmfile->header.order_table[order_pos]].num_rows) {
                row_pos = 0;
                order_pos++;
            }
        }
        xm_channel[0].processSample(obuf, tick_size);
        tick_pos++;
        return tick_size * 4;
    }

private:
    XMFile *xmfile;
};

class XMMixer {
public:
    uint16_t num_chl;
    size_t tick_size;
    std::vector<std::vector<audio16_t>> cbuf;

    void init(XMFile *xmfileRef) {
        xmfile = xmfileRef;
    }

    void setChannel(XMChannel *chl, uint16_t n) {
        channel = chl;
        num_chl = n;
        cbuf.resize(num_chl);
    }

    void setTickSize(size_t tc) {
        tick_size = tc;
        for (uint16_t c = 0; c < num_chl; c++) {
            cbuf[c].resize(tick_size);
        }
    }

    void processTick(audio16_t *abuf, size_t *write_bytes) {

    }

private:
    XMFile *xmfile;
    XMChannel *channel;
};

#endif