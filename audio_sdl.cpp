#include "audio_api.h"
#include <SDL2/SDL.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

struct audio_handle_t {
    SDL_AudioDeviceID device_id;  // SDL Audio Device ID
    SDL_AudioSpec spec;           // SDL Audio Specification
    void* buffer;                 // Audio buffer
    size_t buffer_size;           // Buffer size in bytes
    uint16_t channels;            // Number of channels
    audio_format_t format;        // Sample format
    pthread_mutex_t mutex;        // Mutex for synchronization
    pthread_cond_t cond;          // Condition variable for blocking writes
};

// Initialize the audio system
audio_status_t audio_initialize(audio_handle_t** handle, const audio_init_params_t* params) {
    if (!handle || !params) {
        return AUDIO_INVALID_PARAM;
    }

    if (SDL_Init(SDL_INIT_AUDIO) != 0) {
        return AUDIO_ERROR;
    }

    SDL_AudioSpec desired_spec = {0};
    desired_spec.freq = params->sample_rate;
    desired_spec.channels = params->channels;

    switch (params->format) {
        case AUDIO_FORMAT_U8:
            desired_spec.format = AUDIO_U8;
            break;
        case AUDIO_FORMAT_S16:
            desired_spec.format = AUDIO_S16SYS;
            break;
        case AUDIO_FORMAT_S32:
            desired_spec.format = AUDIO_S32SYS;
            break;
        case AUDIO_FORMAT_FLOAT:
            desired_spec.format = AUDIO_F32SYS;
            break;
        default:
            return AUDIO_INVALID_PARAM;
    }

    desired_spec.samples = (Uint16)(params->buffer_size / (params->channels * SDL_AUDIO_BITSIZE(desired_spec.format) / 8));
    desired_spec.callback = NULL;

    *handle = (audio_handle_t*)malloc(sizeof(audio_handle_t));
    if (!*handle) {
        return AUDIO_ERROR;
    }

    audio_handle_t* h = *handle;
    h->device_id = SDL_OpenAudioDevice(NULL, 0, &desired_spec, &h->spec, 0);
    if (h->device_id == 0) {
        free(h);
        *handle = NULL;
        return AUDIO_ERROR;
    }

    h->buffer = malloc(params->buffer_size);
    if (!h->buffer) {
        SDL_CloseAudioDevice(h->device_id);
        free(h);
        *handle = NULL;
        return AUDIO_ERROR;
    }

    h->buffer_size = params->buffer_size;
    h->channels = params->channels;
    h->format = params->format;

    pthread_mutex_init(&h->mutex, NULL);
    pthread_cond_init(&h->cond, NULL);

    SDL_PauseAudioDevice(h->device_id, 0);  // Start audio playback
    return AUDIO_SUCCESS;
}

// Write audio data with blocking if necessary
audio_status_t audio_write(audio_handle_t* handle, const void* data, size_t data_size) {
    if (!handle || !data || data_size == 0) {
        return AUDIO_INVALID_PARAM;
    }

    pthread_mutex_lock(&handle->mutex);

    while (SDL_GetQueuedAudioSize(handle->device_id) + data_size > handle->buffer_size) {
        pthread_cond_wait(&handle->cond, &handle->mutex);
    }

    int success = SDL_QueueAudio(handle->device_id, data, data_size);
    pthread_mutex_unlock(&handle->mutex);

    return (success == 0) ? AUDIO_SUCCESS : AUDIO_ERROR;
}

// Set new audio parameters
audio_status_t audio_set_params(audio_handle_t* handle, const audio_init_params_t* params) {
    if (!handle || !params) {
        return AUDIO_INVALID_PARAM;
    }

    pthread_mutex_lock(&handle->mutex);

    SDL_CloseAudioDevice(handle->device_id);
    free(handle->buffer);

    SDL_AudioSpec desired_spec = {0};
    desired_spec.freq = params->sample_rate;
    desired_spec.channels = params->channels;

    switch (params->format) {
        case AUDIO_FORMAT_U8:
            desired_spec.format = AUDIO_U8;
            break;
        case AUDIO_FORMAT_S16:
            desired_spec.format = AUDIO_S16SYS;
            break;
        case AUDIO_FORMAT_S32:
            desired_spec.format = AUDIO_S32SYS;
            break;
        case AUDIO_FORMAT_FLOAT:
            desired_spec.format = AUDIO_F32SYS;
            break;
        default:
            pthread_mutex_unlock(&handle->mutex);
            return AUDIO_INVALID_PARAM;
    }

    desired_spec.samples = (Uint16)(params->buffer_size / (params->channels * SDL_AUDIO_BITSIZE(desired_spec.format) / 8));
    desired_spec.callback = NULL;

    SDL_AudioDeviceID new_device_id = SDL_OpenAudioDevice(NULL, 0, &desired_spec, &handle->spec, 0);
    if (new_device_id == 0) {
        pthread_mutex_unlock(&handle->mutex);
        return AUDIO_ERROR;
    }

    void* new_buffer = malloc(params->buffer_size);
    if (!new_buffer) {
        SDL_CloseAudioDevice(new_device_id);
        pthread_mutex_unlock(&handle->mutex);
        return AUDIO_ERROR;
    }

    handle->device_id = new_device_id;
    handle->buffer = new_buffer;
    handle->buffer_size = params->buffer_size;
    handle->channels = params->channels;
    handle->format = params->format;

    SDL_PauseAudioDevice(handle->device_id, 0);

    pthread_mutex_unlock(&handle->mutex);

    return AUDIO_SUCCESS;
}

// Release the audio system resources
audio_status_t audio_release(audio_handle_t* handle) {
    if (!handle) {
        return AUDIO_INVALID_PARAM;
    }

    SDL_CloseAudioDevice(handle->device_id);
    free(handle->buffer);

    pthread_mutex_destroy(&handle->mutex);
    pthread_cond_destroy(&handle->cond);

    free(handle);
    SDL_Quit();

    return AUDIO_SUCCESS;
}
