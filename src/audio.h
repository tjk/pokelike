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
    PaStream *streams[NUM__AUDIO_STREAM];
};

void audio_init(struct audio *audio);

void audio_play(struct audio *audio, enum audio_stream stream);

void audio_destroy(struct audio *audio);
