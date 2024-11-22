#ifndef XMCONTROLLER_H
#define XMCONTROLLER_H

#include <vector>
#include "XMChannel.hpp"
#include "XMTrack.hpp"
#include "XMFile.hpp"

class XMTrack;

class XMController {
public:
    std::vector<XMChannel> xm_channel;
    std::vector<XMTrack> xm_track;
    size_t tick_size;
    uint16_t speed;
    uint16_t tempo;
    uint16_t tick_pos;
    uint16_t row_pos;
    uint16_t order_pos;
    uint16_t chl;

    void init(XMFile* xmfileRef);
    void setTempo(uint16_t tempoRef);
    uint16_t getTempo();
    void setSpeed(uint16_t speedRef);
    uint16_t getSpeed();
    size_t getTickSize();
    size_t processTick(audio16_t* obuf);

private:
    XMFile* xmfile;
};

#endif // XMCONTROLLER_H