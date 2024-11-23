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
        if (param) {
            volSildVal = hexToDecimalTens(param) - hexToDecimalOnes(param);
        }
    }
}

void XMTrack::eraseEffectState() {
    volSild = false;
}

void XMTrack::processVolCtrl(uint8_t vol_cmd) {
    char type;
    uint8_t val;
    parse_vol_cmd(vol_cmd, &type, &val);
    if (type == 'v') {
        channel->setVol(val);
    } else if (type == 'c') {
        volSild = true;
        volSildVal = val;
    } else if (type == 'd') {
        volSild = true;
        volSildVal = -val;
    }
}

void XMTrack::processRows(pattern_cell_t *cell) {
    if (HAS_NOTE(cell->mask)) {
        if (cell->note == 97) {
            channel->noteRelease();
        } else {
            channel->setNote(cell->note);
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

    eraseEffectState();

    if (HAS_VOLUME(cell->mask)) {
        processVolCtrl(cell->vol_ctrl_byte);
    }
    if (HAS_EFFECT_TYPE(cell->mask)) {
        processEffect(cell->effect_type, cell->effect_param);
    }
}

void XMTrack::processTick() {
    if (volSild) {
        channel->setVol(channel->getVol() + volSildVal);
    }
}