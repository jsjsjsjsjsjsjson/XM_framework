#ifndef XMCONTROLLER_H
#define XMCONTROLLER_H

#include <vector>
#include "XMChannel.hpp"
#include "XMTrack.hpp"
#include "XMFile.hpp"
#include "XMMixer.hpp"

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

    void init(XMFile* xmfileRef, XMMixer *xm_mixer);
    void setTempo(uint16_t tempoRef);
    uint16_t getTempo();
    void setSpeed(uint16_t speedRef);
    uint16_t getSpeed();
    size_t getTickSize();
    void processTick();

private:
    XMFile* xmfile;
    XMMixer* mixer;    
};

#endif // XMCONTROLLER_H