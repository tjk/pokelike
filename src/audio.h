#pragma once

#include <portaudio.h>
#include <sndfile.h>

#include "common.h"

PaError err;

enum audio_stream {
    AUDIO_STREAM__BATTLE = 0,

    NUM__AUDIO_STREAM,
};

struct audio {
    SNDFILE *files[NUM__AUDIO_STREAM];
    SNDFILE *current_file; // needed for portaudio callback (called from audio init)
    PaStream *streams[NUM__AUDIO_STREAM];
    PaStream *current_stream; // assume only one stream plays at a time
    bool should_stop; // to force a stop of the stream (and seek back to start)
};

void audio_init(struct audio *audio);

void audio_stop(struct audio *audio);
void audio_play(struct audio *audio, enum audio_stream stream);

void audio_destroy(struct audio *audio);
