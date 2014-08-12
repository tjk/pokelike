#pragma once

#include <portaudio.h>
#include <sndfile.h>

#include "common.h"

PaError err;

typedef enum {
    AUDIO_STREAM__EXPLORE = 0,
    AUDIO_STREAM__BATTLE,

    NUM__AUDIO_STREAM,
} audio_stream_t;

struct audio {
    SNDFILE *files[NUM__AUDIO_STREAM];
    SNDFILE *current_file; // needed for portaudio callback (called from audio init)
    PaStream *streams[NUM__AUDIO_STREAM];
    PaStream *current_stream; // assume only one stream plays at a time
    bool should_stop; // to force a stop of the stream (and seek back to start)
};

void audio_init(struct audio *audio);

void audio_stop(struct audio *audio);
void audio_play(struct audio *audio, audio_stream_t stream);

void audio_destroy(struct audio *audio);
