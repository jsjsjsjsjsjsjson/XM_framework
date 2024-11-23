#include "XMChannel.hpp"

void XMChannel::init(XMController* controllerRef) {
    controller = controllerRef;
}

void XMChannel::setFreq(uint32_t freqRef) {
    freq = freqRef;
    increment = (float)freq / SMP_RATE;
    printf("SET FREQ: %d / %d = %f\n", freq, SMP_RATE, increment);
}

uint32_t XMChannel::getFreq() {
    return freq;
}

void XMChannel::setNote(uint8_t noteRef) {
    note = noteRef;
}

uint8_t XMChannel::getNote() {
    return note;
}

void XMChannel::setVol(int16_t volRef) {
    if (volRef < 0) volRef = 0;
    else if (volRef > 64) volRef = 64;
    vol = volRef;
}

uint16_t XMChannel::getVol() {
    return vol;
}

void XMChannel::setInst(xm_instrument_t *inst) {
    cur_inst = inst;
    vol_envProc.setEnvelope(cur_inst->vol_env, cur_inst->num_vol_point, cur_inst->vol_sus_point,
                                cur_inst->vol_loop_start_point, cur_inst->vol_loop_end_point,
                                    cur_inst->vol_env_type, cur_inst->vol_fadeout);
    printf("SET INST(ENV): POINT = %d, SUS = %d, LOOP = %d ~ %d, TYPEMASK = 0x%X, FADEOUT = %d\n", cur_inst->num_vol_point, cur_inst->vol_sus_point,
                                                                                                    cur_inst->vol_loop_start_point, cur_inst->vol_loop_end_point,
                                                                                                        cur_inst->vol_env_type, cur_inst->vol_fadeout);
    cur_sample = &cur_inst->sample[cur_inst->keymap[note - 1]];
    printf("SET SAMPLE: %d, SAMPLE #%d, VOLUME = %d\n", note, cur_inst->keymap[note], cur_sample->volume);
    setFreq(noteToFrequency(cur_sample->smp_rate, note));
}

xm_instrument_t *XMChannel::getInst() {
    return cur_inst;
}

xm_sample_t *XMChannel::getCurrentSample() {
    return cur_sample;
}

void XMChannel::noteAttack() {
    sample_int_index = 0;
    sample_frac_index = 0;
    samp_state = SAMPLE_PLAYING;
    vol = cur_sample->volume;
    vol_envProc.start();
    printf("SAMPLER(SAMPLE %.22s): START %dHz\n", cur_sample->name, freq);
}

void XMChannel::noteRelease() {
    printf("NOTE RELEASE\n");
    vol_envProc.release();
}

size_t XMChannel::processSample(audio16_t *buf, size_t tick_size) {
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

size_t XMChannel::processTick(audio16_t *buf, size_t tick_size) {
    vol_envProc.next();
    env_vol = vol_envProc.getValue();
    return processSample(buf, tick_size);
}