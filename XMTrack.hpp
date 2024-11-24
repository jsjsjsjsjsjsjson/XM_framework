#ifndef XMTRACK_H
#define XMTRACK_H

#include <stdbool.h>
#include "XMFile.hpp"
#include "XMController.hpp"

class XMFile;
class XMController;
class XMChannel;

#define NO_NOTE 255
#define NOTE_OFF 97

#define SILD_SCALE 128

class XMTrack {
public:
    bool volSild = false;
    int8_t volSildVal = 0;

    bool freqSildUp = false;
    uint8_t freqSildUpVal = 0;

    bool freqSildDown = false;
    uint8_t freqSildDownVal = 0;

    bool portToNote = false;
    bool portToNoteActv = false;
    uint8_t portToNoteSpeed = 0;
    int32_t portToNoteCur = 0;
    int32_t portToNoteSource = 0;
    int32_t portToNoteTarget = 0;

    void eraseEffectState();
    void init(XMFile* xmfileRef, XMController* controllerRef, XMChannel* channelRef);
    void processEffect(uint8_t type, uint8_t param);
    void processVolCtrl(uint8_t val);
    void processRows(pattern_cell_t* cell);
    void processTick();

private:
    uint8_t cur_note;
    XMFile* xmfile;
    XMController* controller;
    XMChannel* channel;
};

#endif // XMTRACK_H