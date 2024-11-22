#include "XMMixer.hpp"

// void XMMixer::init(XMFile *xmfileRef) {
//     xmfile = xmfileRef;
// }

void XMMixer::setChannel(std::vector<XMChannel> *chl) {
    channel = chl;
    cbuf.resize(channel->size());
}

void XMMixer::setTickSize(size_t tc) {
    tick_size = tc;
    for (uint16_t c = 0; c < channel->size(); c++) {
        cbuf[c].resize(tick_size);
    }
}

void XMMixer::processTick(audio16_t *abuf, size_t *write_bytes) {
    for (uint16_t c = 0; c < channel->size(); c++) {
        (*channel)[c].processTick(cbuf[c].data(), tick_size);
    }
    for (size_t t = 0; t < tick_size; t++) {
        audio32_t sum = {0, 0};
        for (uint16_t c = 0; c < channel->size(); c++) {
            sum.l += cbuf[c][t].l;
            sum.r += cbuf[c][t].r;
        }
        abuf[t].l = sum.l / (int32_t)(channel->size());
        abuf[t].r = sum.r / (int32_t)(channel->size());
    }
    *write_bytes = tick_size * 4;
}