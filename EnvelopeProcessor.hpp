#ifndef ENVELOPE_PROCESSOR_HPP
#define ENVELOPE_PROCESSOR_HPP

#include <stdint.h>
#include <stddef.h>

typedef enum {
    ENV_PLAYING,
    ENV_SUSTAINING,
    ENV_RELEASING,
    ENV_FADINGOUT,
    ENV_FINISHED
} env_state_t;

typedef struct {
    bool on : 1;
    bool sus : 1;
    bool loop : 1;
} env_type_t;

typedef struct __attribute__((packed)) {
    uint16_t x;
    uint16_t y;
} env_point_t;

class EnvelopeProcessor {
public:
    EnvelopeProcessor();

    void setEnvelope(const env_point_t* env, uint8_t points, uint8_t sus, uint8_t loopStart, uint8_t loopEnd, env_type_t type, uint16_t fadeout);
    void reset();
    void start();
    void release();
    bool next();

    uint16_t getValue() const;
    env_state_t getStatus() const;

private:
    const env_point_t* envelope;
    uint8_t num_points;
    uint8_t sustain_point;
    uint8_t loop_start;
    uint8_t loop_end;
    env_type_t env_type;
    uint16_t vol_fadeout;

    env_state_t state;
    uint16_t current_x;
    uint16_t current_y;
    uint32_t fadeout_value;

    float incremental_y;
    uint16_t segment_start_x;
    uint16_t segment_end_x;
    bool continuous_x;

    void updateIncremental(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
    size_t findEnvelopeSegment();
};

const char* getStateName(env_state_t state);

#endif // ENVELOPE_PROCESSOR_HPP
