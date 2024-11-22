#include "XMFile.hpp"

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

void XMFile::open_xm_file(const char *filename) {
    xm_file = fopen(filename, "rb+");
    current_file_name = (char*)realloc(current_file_name, strlen(filename));
    strcpy(current_file_name, filename);
}

void XMFile::close_xm_file() {
    fclose(xm_file);
}

void XMFile::read_xm_header() {
    fseek(xm_file, 0, SEEK_SET);
    fread(&header, 1, 80, xm_file);
    header.order_table.resize(header.song_len);
    fread(header.order_table.data(), 1, header.song_len, xm_file);
    for (uint8_t i = 0; i < header.song_len; i++) {
        printf("%d ", header.order_table[i]);
    }
    printf("\n");
}

void XMFile::read_xm_pattern() {
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

void XMFile::read_xm_sample(xm_instrument_t *inst) {
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

void XMFile::read_xm_instrument() {
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

void XMFile::print_pattern(uint16_t num, int startChl, int endChl, int startRow, int endRow) {
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

void XMFile::print_xm_info() {
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

void XMFile::load_xm_file() {
    read_xm_header();
    read_xm_pattern();
    read_xm_instrument();
}