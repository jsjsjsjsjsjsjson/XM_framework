#include "XMTrack.hpp"

void XMTrack::init(XMFile *xmfileRef, XMController *controllerRef, XMChannel *channelRef) {
    xmfile = xmfileRef;
    controller = controllerRef;
    channel = channelRef;
}

void XMTrack::processEffect(uint8_t type, uint8_t param) {
    if (type == 0xF) {
        if (param < 32) {
            controller->setTempo(param);
            printf("SET TEMPO: %d\n", param);
        } else {
            controller->setSpeed(param);
            printf("SET SPEED: %d\n", param);
        }
    } else if (type == 0xA) {
        volSild = true;
        // if (hexToDecimalTens(param)) {
        //     volSildVal = 
        // } else if (hexToDecimalOnes(param)) {

        // } else {
        //     ?
        // }
        if (param) {
            volSildVal = hexToDecimalTens(param) - hexToDecimalOnes(param);
        }
    }
}

void XMTrack::processRows(pattern_cell_t *cell) {
    if (HAS_NOTE(cell->mask)) {
        channel->setNote(cell->note);
        if (cell->note == 97) {
            channel->noteRelease();
        }
    }
    if (HAS_INSTRUMENT(cell->mask)) {
        channel->setInst(&xmfile->instrument[cell->instrument - 1]);
        if (channel->getNote() == 97) {
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
