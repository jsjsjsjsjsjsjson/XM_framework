#ifndef AUDIO_API_H
#define AUDIO_API_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

// Audio API return codes
typedef enum {
    AUDIO_SUCCESS = 0,
    AUDIO_ERROR = -1,
    AUDIO_INVALID_PARAM = -2,
    AUDIO_BUFFER_FULL = -3,
    AUDIO_UNINITIALIZED = -4
} audio_status_t;

// Audio sample formats
typedef enum {
    AUDIO_FORMAT_U8 = 0,    // 8-bit unsigned integer
    AUDIO_FORMAT_S16 = 1,   // 16-bit signed integer
    AUDIO_FORMAT_S32 = 2,   // 32-bit signed integer
    AUDIO_FORMAT_FLOAT = 3  // 32-bit floating-point
} audio_format_t;

// Audio API handle
typedef struct audio_handle_t audio_handle_t;

// Initialization parameters
typedef struct {
    uint32_t sample_rate;    // Sampling rate (e.g., 44100 Hz)
    uint16_t channels;       // Number of channels (e.g., 1 for mono, 2 for stereo)
    audio_format_t format;   // Sample format
    size_t buffer_size;      // Buffer size in bytes
} audio_init_params_t;

// Function to initialize the audio system
audio_status_t audio_initialize(audio_handle_t** handle, const audio_init_params_t* params);

// Function to write audio data
audio_status_t audio_write(audio_handle_t* handle, const void* data, size_t data_size);

// Function to modify audio parameters (e.g., sample rate, format)
audio_status_t audio_set_params(audio_handle_t* handle, const audio_init_params_t* params);

// Function to release the audio system resources
audio_status_t audio_release(audio_handle_t* handle);

#ifdef __cplusplus
}
#endif

#endif