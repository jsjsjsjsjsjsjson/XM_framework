#include "XMTrack.hpp"
#include <math.h>

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
    } else if (type == 0x1) {
        freqSildUp = true;
        if (param) freqSildUpVal = param;
    } else if (type == 0x2) {
        freqSildDown = true;
        if (param) freqSildDownVal = param;
    } else if (type == 0x3) {
        portToNote = true;
        if (param) {
            portToNoteSpeed = param;
        }
        if (cur_note != NO_NOTE) {
            portToNoteActv = true;
            portToNoteSource = channel->getLastFreq();
            portToNoteTarget = channel->getFreq();
            portToNoteCur = portToNoteSource;
            channel->setFreq(portToNoteCur);
            printf("SET PORT %d TO %d, SPEED: %d\n", portToNoteSource, portToNoteTarget, portToNoteSpeed);
        }
    }
}

void XMTrack::eraseEffectState() {
    freqSildUp = false;
    freqSildDown = false;
    portToNote = false;
    portToNoteActv = false;
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
    } else if (type == 'g') {
        portToNote = true;
        if (val) {
            portToNoteSpeed = val;
        }
        if (cur_note != NO_NOTE) {
            portToNoteActv = true;
            portToNoteSource = channel->getLastFreq();
            portToNoteTarget = channel->getFreq();
            portToNoteCur = portToNoteSource;
            channel->setFreq(portToNoteCur);
            printf("SET PORT %d TO %d, SPEED: %d\n", portToNoteSource, portToNoteTarget, portToNoteSpeed);
        }
    }
}

void XMTrack::processRows(pattern_cell_t *cell) {
    if (HAS_INSTRUMENT(cell->mask)) {
        channel->setInst(&xmfile->instrument[cell->instrument - 1], false);
        if (channel->getNote() == NOTE_OFF) {
            channel->noteRelease();
        } else {
            channel->noteAttack(!(cell->effect_type == 0x3));
        }
    }
    if (HAS_NOTE(cell->mask)) {
        cur_note = cell->note;
        if (cell->note == NOTE_OFF) {
            channel->noteRelease();
        } else {
            channel->setNote(cell->note, false);
        }
    } else {
        cur_note = NO_NOTE;
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
    if (freqSildUp) {
        channel->setFreq(PORTUP(channel->getFreq(), freqSildUpVal, SILD_SCALE * 2));
    }
    if (freqSildDown) {
        channel->setFreq(PORTDOWN(channel->getFreq(), freqSildDownVal, SILD_SCALE * 2));
    }
    if (portToNote && portToNoteActv) {
        if (portToNoteTarget > portToNoteCur) {
            portToNoteCur = PORTUP(portToNoteCur, portToNoteSpeed, SILD_SCALE / 3);
            if (portToNoteCur >= portToNoteTarget) {
                portToNoteCur = portToNoteTarget;
                portToNoteActv = false;
            }
        } else if (portToNoteTarget < portToNoteCur) {
            portToNoteCur = PORTDOWN(portToNoteCur, portToNoteSpeed, SILD_SCALE);
            if (portToNoteCur <= portToNoteTarget) {
                portToNoteCur = portToNoteTarget;
                portToNoteActv = false;
            }
        }
        if (portToNoteCur < 0) portToNoteCur = 0;
        channel->setFreq(portToNoteCur);

        printf("Sliding: Current Freq = %d, Target Freq = %d\n", portToNoteCur, portToNoteTarget);
    }
}