#include "audio_api.h"
#include <portaudio.h>
#include <stdlib.h>
#include <string.h>

struct audio_handle_t {
    PaStream* stream;
};

// Helper function to convert audio format enum to PortAudio sample format
static PaSampleFormat get_sample_format(audio_format_t format) {
    switch (format) {
        case AUDIO_FORMAT_U8: return paUInt8;
        case AUDIO_FORMAT_S16: return paInt16;
        case AUDIO_FORMAT_S32: return paInt32;
        case AUDIO_FORMAT_FLOAT: return paFloat32;
        default: return paCustomFormat; // This should be handled properly
    }
}

// Callback function used by PortAudio engine
static int paCallback(const void *inputBuffer, void *outputBuffer,
                      unsigned long framesPerBuffer,
                      const PaStreamCallbackTimeInfo* timeInfo,
                      PaStreamCallbackFlags statusFlags,
                      void *userData) {
    // For simplicity, we are not handling input buffer and assuming output only
    (void) inputBuffer; // Prevent unused variable warnings
    (void) timeInfo;
    (void) statusFlags;
    (void) userData;
    
    memset(outputBuffer, 0, framesPerBuffer * sizeof(float)); // Example: clear buffer
    return paContinue;
}

audio_status_t audio_initialize(audio_handle_t** handle, const audio_init_params_t* params) {
    if (!handle || !params) return AUDIO_INVALID_PARAM;

    *handle = (audio_handle_t*)malloc(sizeof(audio_handle_t));
    if (!*handle) return AUDIO_ERROR;

    PaError err;
    err = Pa_Initialize();
    if (err != paNoError) return AUDIO_ERROR;

    PaStreamParameters outputParameters;
    outputParameters.device = Pa_GetDefaultOutputDevice();
    outputParameters.channelCount = params->channels;
    outputParameters.sampleFormat = get_sample_format(params->format);
    outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = NULL;

    err = Pa_OpenStream(&(*handle)->stream, NULL, &outputParameters,
                        params->sample_rate, params->buffer_size / (params->channels * sizeof(float)),
                        paClipOff, paCallback, NULL);
    if (err != paNoError) {
        free(*handle);
        return AUDIO_ERROR;
    }

    err = Pa_StartStream((*handle)->stream);
    if (err != paNoError) {
        Pa_CloseStream((*handle)->stream);
        free(*handle);
        return AUDIO_ERROR;
    }

    return AUDIO_SUCCESS;
}

audio_status_t audio_write(audio_handle_t* handle, const void* data, size_t data_size) {
    if (!handle || !handle->stream) return AUDIO_UNINITIALIZED;

    PaError err = Pa_WriteStream(handle->stream, data, data_size / sizeof(float));
    if (err != paNoError) return AUDIO_ERROR;

    return AUDIO_SUCCESS;
}

audio_status_t audio_set_params(audio_handle_t* handle, const audio_init_params_t* params) {
    // Reinitializing the stream with new parameters
    if (!handle || !params) return AUDIO_INVALID_PARAM;

    audio_release(handle);
    return audio_initialize(&handle, params);
}

audio_status_t audio_release(audio_handle_t* handle) {
    if (!handle) return AUDIO_UNINITIALIZED;

    Pa_StopStream(handle->stream);
    Pa_CloseStream(handle->stream);
    Pa_Terminate();
    free(handle);

    return AUDIO_SUCCESS;
}
