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
    xm_sample_t* cur_sample;
    samp_state_t samp_state;
    xm_instrument_t* cur_inst;
    void init(XMController* controllerRef);
    void setFreq(int32_t freqRef);
    uint32_t getFreq();
    uint32_t getLastFreq();
    void setNote(uint8_t noteRef, bool rst_freq);
    uint8_t getNote();
    uint8_t getLastNote();
    void setVol(int16_t volRef);
    uint16_t getVol();
    void setInst(xm_instrument_t *inst, bool rst_freq);
    xm_instrument_t* getInst();
    xm_sample_t* getCurrentSample();
    void noteAttack(bool rst);
    void noteRelease();
    size_t processSample(audio16_t* buf, size_t tick_size);
    size_t processTick(audio16_t* buf, size_t tick_size);

private:
    XMController* controller;
    EnvelopeProcessor vol_envProc;
    EnvelopeProcessor pan_envProc;
    uint32_t lastFreq;
    uint32_t freq;
    uint8_t note;
    uint8_t lastNote;
    uint16_t vol;
    float sample_frac_index;
    uint16_t env_vol;
    uint32_t sample_int_index;
    float increment;
};

#endif // XMCHANNEL_H