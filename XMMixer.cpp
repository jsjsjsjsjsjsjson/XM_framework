#include "XMMixer.hpp"

void XMMixer::init(XMFile *xmfileRef) {
    xmfile = xmfileRef;
}

void XMMixer::setChannel(XMChannel *chl, uint16_t n) {
    channel = chl;
    num_chl = n;
    cbuf.resize(num_chl);
}

void XMMixer::setTickSize(size_t tc) {
    tick_size = tc;
    for (uint16_t c = 0; c < num_chl; c++) {
        cbuf[c].resize(tick_size);
    }
}

void XMMixer::processTick(audio16_t *abuf, size_t *write_bytes) {
    
}