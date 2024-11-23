#include "XMController.hpp"

void XMController::init(XMFile *xmfileRef, XMMixer *xm_mixer) {
    xmfile = xmfileRef;
    mixer = xm_mixer;
    printf("BPM: %d\n", xmfile->header.def_bpm);

    xm_channel.resize(xmfile->header.num_channel);
    xm_track.resize(xmfile->header.num_channel);
    for (uint16_t i = 0; i < xmfile->header.num_channel; i++) {
        xm_channel[i].init(this);
        xm_track[i].init(xmfile, this, &xm_channel[i]);
    }

    mixer->setChannel(&xm_channel);
    
    speed = xmfile->header.def_bpm;
    tempo = xmfile->header.def_tempo;
    tick_size = bpmToTicksize(speed, SMP_RATE);
    tick_pos = tempo;
    order_pos = xmfile->header.restart_pos;
    mixer->setTickSize(tick_size);
}

void XMController::setTempo(uint16_t tempoRef) {
    tempo = tempoRef;
    tick_pos = tempo;
}

uint16_t XMController::getTempo() {
    return tempo;
}

void XMController::setSpeed(uint16_t speedRef) {
    speed = speedRef;
    tick_size = bpmToTicksize(speed, SMP_RATE);
    printf("SET BPM: %d, TICKSIZE: %ld\n", speed, tick_size);
    mixer->setTickSize(tick_size);
}

uint16_t XMController::getSpeed() {
    return speed;
}

size_t XMController::getTickSize() {
    return tick_size;
}

void XMController::processTick() {
    if (tick_pos >= tempo) {
        // printf("TICK_POS: %d, TEMPO: %d\n", tick_pos, tempo);
        for (uint16_t c = 0; c < xmfile->header.num_channel; c++) {
            xm_track[c].processRows(&xmfile->pattern[xmfile->header.order_table[order_pos]].unpack_data[c][row_pos]);
        }
        tick_pos = 0;
        row_pos++;
        if (row_pos >= xmfile->pattern[xmfile->header.order_table[order_pos]].num_rows) {
            row_pos = 0;
            order_pos++;
        }
    } else {
        for (uint16_t c = 0; c < xmfile->header.num_channel; c++) {
            xm_track[c].processTick();
        }
    }
    tick_pos++;
}