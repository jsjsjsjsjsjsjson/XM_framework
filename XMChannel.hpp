#ifndef XMCHANNEL_H
#define XMCHANNEL_H

#include "XMFile.hpp"
#include "EnvelopeProcessor.hpp"
#include "audio_struct.h"

typedef enum {
    SAMPLE_STOP,
    SAMPLE_PLAYING
} samp_state_t;

class XMController;

class XMChannel {
public:
    void init(XMController* controllerRef);
    void setFreq(uint32_t freqRef);
    uint32_t getFreq();
    void setNote(uint8_t noteRef);
    uint8_t getNote();
    void setVol(uint16_t volRef);
    uint16_t getVol();
    void setInst(xm_instrument_t *inst);
    xm_instrument_t* getInst();
    xm_sample_t* getCurrentSample();
    void noteAttack();
    void noteRelease();
    size_t processSample(audio16_t* buf, size_t tick_size);
    size_t processTick(audio16_t* buf, size_t tick_size);

private:
    XMController* controller;
    EnvelopeProcessor vol_envProc;
    EnvelopeProcessor pan_envProc;
    uint32_t freq;
    uint8_t note;
    uint16_t vol;
    float sample_frac_index;
    uint16_t env_vol;
    uint32_t sample_int_index;
    float increment;
    samp_state_t samp_state;
    xm_instrument_t* cur_inst;
    xm_sample_t* cur_sample;
};

#endif // XMCHANNEL_H