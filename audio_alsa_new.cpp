#include "audio_api.h"
#include <alsa/asoundlib.h>
#include <stdlib.h>

// Definition of the audio handle structure
struct audio_handle_t {
    snd_pcm_t *pcm_handle;
    snd_pcm_hw_params_t *params;
    snd_pcm_format_t alsa_format;
};

// Helper function to translate API format to ALSA format
static snd_pcm_format_t translate_format(audio_format_t format) {
    switch (format) {
        case AUDIO_FORMAT_U8:    return SND_PCM_FORMAT_U8;
        case AUDIO_FORMAT_S16:   return SND_PCM_FORMAT_S16_LE;
        case AUDIO_FORMAT_S32:   return SND_PCM_FORMAT_S32_LE;
        case AUDIO_FORMAT_FLOAT: return SND_PCM_FORMAT_FLOAT_LE;
        default:                 return SND_PCM_FORMAT_UNKNOWN;
    }
}

// Initializes the audio system
audio_status_t audio_initialize(audio_handle_t** handle, const audio_init_params_t* params) {
    if (!handle || !params) return AUDIO_INVALID_PARAM;

    *handle = (audio_handle_t*)calloc(1, sizeof(audio_handle_t));
    if (!*handle) return AUDIO_ERROR;

    int err = snd_pcm_open(&((*handle)->pcm_handle), "default", SND_PCM_STREAM_PLAYBACK, 0);
    if (err < 0) {
        free(*handle);
        return AUDIO_ERROR;
    }

    snd_pcm_hw_params_malloc(&((*handle)->params));
    snd_pcm_hw_params_any((*handle)->pcm_handle, (*handle)->params);
    snd_pcm_hw_params_set_rate((*handle)->pcm_handle, (*handle)->params, params->sample_rate, 0);
    snd_pcm_hw_params_set_channels((*handle)->pcm_handle, (*handle)->params, params->channels);
    (*handle)->alsa_format = translate_format(params->format);
    snd_pcm_hw_params_set_format((*handle)->pcm_handle, (*handle)->params, (*handle)->alsa_format);
    snd_pcm_hw_params_set_buffer_size((*handle)->pcm_handle, (*handle)->params, params->buffer_size / (params->channels * snd_pcm_format_physical_width((*handle)->alsa_format) / 8));

    if (snd_pcm_hw_params((*handle)->pcm_handle, (*handle)->params) < 0) {
        snd_pcm_close((*handle)->pcm_handle);
        free(*handle);
        return AUDIO_ERROR;
    }

    snd_pcm_prepare((*handle)->pcm_handle);

    return AUDIO_SUCCESS;
}

// Writes audio data
audio_status_t audio_write(audio_handle_t* handle, const void* data, size_t data_size) {
    if (!handle || !data) return AUDIO_INVALID_PARAM;

    int frames = data_size / (snd_pcm_format_physical_width(handle->alsa_format) / 8);
    int err = snd_pcm_writei(handle->pcm_handle, data, frames);
    if (err < 0) {
        snd_pcm_prepare(handle->pcm_handle); // Attempt to recover from an xrun
        return AUDIO_ERROR;
    }
    return AUDIO_SUCCESS;
}

// Sets audio parameters
audio_status_t audio_set_params(audio_handle_t* handle, const audio_init_params_t* new_params) {
    if (!handle || !new_params) return AUDIO_INVALID_PARAM;

    // It's often simpler and more reliable to close and reopen the device with new parameters
    snd_pcm_close(handle->pcm_handle);
    return audio_initialize(&handle, new_params);
}

// Releases audio system resources
audio_status_t audio_release(audio_handle_t* handle) {
    if (!handle) return AUDIO_INVALID_PARAM;

    snd_pcm_close(handle->pcm_handle);
    free(handle);
    return AUDIO_SUCCESS;
}
