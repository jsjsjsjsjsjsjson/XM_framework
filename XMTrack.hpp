#ifndef XMTRACK_H
#define XMTRACK_H

#include <stdbool.h>
#include "XMFile.hpp"
#include "XMController.hpp"

class XMFile;
class XMController;
class XMChannel;

class XMTrack {
public:
    bool volSild = false;
    int8_t volSildVal = 0;

    void eraseEffectState();
    void init(XMFile* xmfileRef, XMController* controllerRef, XMChannel* channelRef);
    void processEffect(uint8_t type, uint8_t param);
    void processVolCtrl(uint8_t val);
    void processRows(pattern_cell_t* cell);
    void processTick();

private:
    XMFile* xmfile;
    XMController* controller;
    XMChannel* channel;
};

#endif // XMTRACK_H