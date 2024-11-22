#include "XMController.hpp"

void XMController::init(XMFile *xmfileRef) {
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
    order_pos = 2;
    chl = 9;
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
    tick_size = bpmToTicksize(xmfile->header.def_bpm, SMP_RATE);
}

uint16_t XMController::getSpeed() {
    return speed;
}

size_t XMController::getTickSize() {
    return tick_size;
}

size_t XMController::processTick(audio16_t *obuf) {
    if (tick_pos >= tempo) {
        pattern_cell_t *cell = &xmfile->pattern[xmfile->header.order_table[order_pos]].unpack_data[chl][row_pos];
        printf("%02d, %02d, %02d: %03d %02d\n", order_pos, chl, row_pos, cell->note, cell->instrument);
        xm_track[chl].processRows(cell);
        tick_pos = 0;
        row_pos++;
        if (row_pos >= xmfile->pattern[xmfile->header.order_table[order_pos]].num_rows) {
            row_pos = 0;
            order_pos++;
        }
    }
    xm_channel[chl].processSample(obuf, tick_size);
    tick_pos++;
    return tick_size * 4;
}