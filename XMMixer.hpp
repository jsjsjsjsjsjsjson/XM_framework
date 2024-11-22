#ifndef XMMIXER_H
#define XMMIXER_H

#include <stdio.h>
#include <vector>
#include "audio_struct.h"
#include "XMChannel.hpp"

class XMFile;
class XMChannel;

class XMMixer {
public:
    // void init(XMFile* xmfileRef);
    void setChannel(std::vector<XMChannel>* chl);
    void setTickSize(size_t tc);
    void processTick(audio16_t* abuf, size_t* write_bytes);

private:
    XMFile* xmfile;
    std::vector<XMChannel> *channel;
    size_t tick_size;
    std::vector<std::vector<audio16_t>> cbuf;
};

#endif // XMMIXER_H
