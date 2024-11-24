#ifndef AUDIO_STRUCT_H
#define AUDIO_STRUCT_H

#include <stdint.h>

typedef struct {
    int32_t l;
    int32_t r;
} audio32_t;

typedef struct {
    int16_t l;
    int16_t r;
} audio16_t;

typedef struct {
    int8_t l;
    int8_t r;
} audio8_t;

typedef int16_t audio16_mono_t;
typedef int8_t audio8_mono_t;

#define SMP_RATE 96000
#define BUFF_SIZE 4096

#endif