#ifndef XMTRACK_H
#define XMTRACK_H

#include "XMFile.hpp"
#include "XMController.hpp"

class XMFile;
class XMController;
class XMChannel;

class XMTrack {
public:
    void init(XMFile* xmfileRef, XMController* controllerRef, XMChannel* channelRef);
    void processEffect(uint8_t type, uint8_t param);
    void processRows(pattern_cell_t* cell);

private:
    XMFile* xmfile;
    XMController* controller;
    XMChannel* channel;
};

#endif // XMTRACK_H