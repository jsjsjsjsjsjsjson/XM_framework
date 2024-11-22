#include "EnvelopeProcessor.hpp"

EnvelopeProcessor::EnvelopeProcessor()
    : envelope(nullptr), num_points(0), sustain_point(0), loop_start(0), loop_end(0),
      vol_fadeout(0), state(ENV_FINISHED), current_x(0),
      current_y(64), fadeout_value(65536), incremental_y(0.0f),
      segment_start_x(0), segment_end_x(0), continuous_x(false) {}

void EnvelopeProcessor::setEnvelope(const env_point_t* env, uint8_t points, uint8_t sus, uint8_t loopStart, uint8_t loopEnd, env_type_t type, uint16_t fadeout) {
    envelope = env;
    num_points = points;
    sustain_point = sus;
    loop_start = loopStart;
    loop_end = loopEnd;
    env_type = type;
    vol_fadeout = fadeout;
    reset();
}

void EnvelopeProcessor::reset() {
    state = ENV_PLAYING;
    current_x = 0;
    current_y = env_type.on ? (envelope ? envelope[0].y : 0) : 64;
    fadeout_value = 65536;
    incremental_y = 0.0f;
    segment_start_x = 0;
    segment_end_x = 0;
    continuous_x = false;
}

void EnvelopeProcessor::start() {
    reset();
}

void EnvelopeProcessor::release() {
    if (state == ENV_PLAYING || state == ENV_SUSTAINING) {
        state = ENV_RELEASING;
    }
}

bool EnvelopeProcessor::next() {
    if (state == ENV_FINISHED) {
        current_y = 0;
        return false;
    }

    if (!env_type.on) {
        if (state == ENV_RELEASING) {
            fadeout_value = (fadeout_value > vol_fadeout) ? (fadeout_value - vol_fadeout) : 0;
            current_y = (64 * fadeout_value) >> 16;

            if (fadeout_value == 0) {
                state = ENV_FINISHED;
                current_y = 0;
            }
            return true;
        }

        current_y = 64;
        return true;
    }

    if (state == ENV_FADINGOUT) {
        fadeout_value = (fadeout_value > vol_fadeout) ? (fadeout_value - vol_fadeout) : 0;
        current_y = (current_y * fadeout_value) >> 16;

        if (fadeout_value == 0) {
            state = ENV_FINISHED;
            current_y = 0;
        }
        return true;
    }

    if (state == ENV_RELEASING) {
        size_t segment = findEnvelopeSegment();
        uint16_t x0 = envelope[segment].x;
        uint16_t y0 = envelope[segment].y;
        uint16_t x1 = envelope[segment + 1].x;
        uint16_t y1 = envelope[segment + 1].y;

        if (current_x != segment_start_x) {
            updateIncremental(x0, y0, x1, y1);
        }

        current_y = y0 + (uint16_t)(incremental_y * (current_x - x0));
        current_x++;

        if (current_x >= envelope[num_points - 1].x) {
            state = ENV_FADINGOUT;
        }

        fadeout_value = (fadeout_value > vol_fadeout) ? (fadeout_value - vol_fadeout) : 0;
        current_y = (current_y * fadeout_value) >> 16;

        if (fadeout_value == 0) {
            state = ENV_FINISHED;
            current_y = 0;
        }
        return true;
    }

    if (state == ENV_SUSTAINING) {
        current_y = envelope[sustain_point].y;
        return true;
    }

    if (state == ENV_PLAYING && env_type.sus && current_x >= envelope[sustain_point].x) {
        state = ENV_SUSTAINING;
        current_y = envelope[sustain_point].y;
        return true;
    }

    size_t segment = findEnvelopeSegment();
    uint16_t x0 = envelope[segment].x;
    uint16_t y0 = envelope[segment].y;
    uint16_t x1 = envelope[segment + 1].x;
    uint16_t y1 = envelope[segment + 1].y;

    if (current_x != segment_start_x) {
        updateIncremental(x0, y0, x1, y1);
    }

    current_y = y0 + (uint16_t)(incremental_y * (current_x - x0));
    current_x++;

    if (env_type.loop && state == ENV_PLAYING && current_x >= envelope[loop_end].x) {
        current_x = envelope[loop_start].x;
    }

    if (current_x >= envelope[num_points - 1].x) {
        state = ENV_FADINGOUT;
    }

    return state != ENV_FINISHED;
}

uint16_t EnvelopeProcessor::getValue() const {
    return current_y;
}

env_state_t EnvelopeProcessor::getStatus() const {
    return state;
}

void EnvelopeProcessor::updateIncremental(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    segment_start_x = x0;
    segment_end_x = x1;
    if (x1 > x0) {
        incremental_y = (float)(y1 - y0) / (x1 - x0);
    } else {
        incremental_y = 0.0f;
    }
    continuous_x = true;
}

size_t EnvelopeProcessor::findEnvelopeSegment() {
    for (size_t i = 0; i < num_points - 1; ++i) {
        if (current_x >= envelope[i].x && current_x < envelope[i + 1].x) {
            return i;
        }
    }
    return num_points - 1;
}

const char* getStateName(env_state_t state) {
    switch (state) {
        case ENV_PLAYING:    return "Playing";
        case ENV_SUSTAINING: return "Sustaining";
        case ENV_RELEASING:  return "Releasing";
        case ENV_FADINGOUT:  return "FadingOut";
        case ENV_FINISHED:   return "Finished";
        default:             return "Unknown";
    }
}