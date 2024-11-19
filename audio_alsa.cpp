#include "audio_api.h"
#include <alsa/asoundlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct audio_handle_t {
    snd_pcm_t *pcm_handle;
    audio_init_params_t params;
};

audio_status_t audio_initialize(audio_handle_t **handle, const audio_init_params_t *params) {
    if (!handle || !params) return AUDIO_INVALID_PARAM;

    *handle = (audio_handle_t*)malloc(sizeof(audio_handle_t));
    if (!*handle) return AUDIO_ERROR;

    (*handle)->params = *params;
    snd_pcm_hw_params_t *hw_params;
    snd_pcm_t *pcm;
    int err;

    if ((err = snd_pcm_open(&pcm, "default", SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
        free(*handle);
        *handle = NULL;
        return AUDIO_ERROR;
    }

    snd_pcm_hw_params_alloca(&hw_params);
    snd_pcm_hw_params_any(pcm, hw_params);
    snd_pcm_hw_params_set_access(pcm, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
    
    snd_pcm_format_t alsa_format;
    switch (params->format) {
        case AUDIO_FORMAT_U8:
            alsa_format = SND_PCM_FORMAT_U8;
            break;
        case AUDIO_FORMAT_S16:
            alsa_format = SND_PCM_FORMAT_S16_LE;
            break;
        case AUDIO_FORMAT_S32:
            alsa_format = SND_PCM_FORMAT_S32_LE;
            break;
        case AUDIO_FORMAT_FLOAT:
            alsa_format = SND_PCM_FORMAT_FLOAT_LE;
            break;
        default:
            snd_pcm_close(pcm);
            free(*handle);
            *handle = NULL;
            return AUDIO_INVALID_PARAM;
    }

    snd_pcm_hw_params_set_format(pcm, hw_params, alsa_format);
    snd_pcm_hw_params_set_channels(pcm, hw_params, params->channels);
    snd_pcm_hw_params_set_rate(pcm, hw_params, params->sample_rate, 0);

    if ((err = snd_pcm_hw_params(pcm, hw_params)) < 0) {
        snd_pcm_close(pcm);
        free(*handle);
        *handle = NULL;
        return AUDIO_ERROR;
    }

    (*handle)->pcm_handle = pcm;
    return AUDIO_SUCCESS;
}

audio_status_t audio_write(audio_handle_t *handle, const void *data, size_t data_size) {
    if (!handle || !data || !handle->pcm_handle) return AUDIO_UNINITIALIZED;

    snd_pcm_sframes_t frames = snd_pcm_writei(handle->pcm_handle, data, data_size / (handle->params.channels * (snd_pcm_format_width(SND_PCM_FORMAT_S16_LE) / 8)));
    if (frames < 0) {
        frames = snd_pcm_recover(handle->pcm_handle, frames, 0);
        if (frames < 0) return AUDIO_ERROR;
    }

    return AUDIO_SUCCESS;
}

audio_status_t audio_set_params(audio_handle_t *handle, const audio_init_params_t *params) {
    if (!handle || !params) return AUDIO_INVALID_PARAM;

    audio_release(handle);
    return audio_initialize(&handle, params);
}

audio_status_t audio_release(audio_handle_t *handle) {
    if (!handle) return AUDIO_INVALID_PARAM;

    if (handle->pcm_handle) {
        snd_pcm_drain(handle->pcm_handle);
        snd_pcm_close(handle->pcm_handle);
    }

    free(handle);
    return AUDIO_SUCCESS;
}
