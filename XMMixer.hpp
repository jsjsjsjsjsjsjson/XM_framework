#ifndef XMMIXER_H
#define XMMIXER_H

#include <stdio.h>
#include <vector>
#include "audio_struct.h"

class XMFile;
class XMChannel;

class XMMixer {
public:
    void init(XMFile* xmfileRef);
    void setChannel(XMChannel* chl, uint16_t n);
    void setTickSize(size_t tc);
    void processTick(audio16_t* abuf, size_t* write_bytes);

private:
    XMFile* xmfile;
    XMChannel* channel;
    uint16_t num_chl;
    size_t tick_size;
    std::vector<std::vector<audio16_t>> cbuf;
};

#endif // XMMIXER_H
