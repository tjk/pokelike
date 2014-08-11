#include "audio.h"

// must match with enum audio_stream
static char *audio_stream_paths[] = {
    "./assets/battle.wav",
};

static int _audio_sndfile_cb(const void *input, void *output, unsigned long frames_per_buffer,
        const PaStreamCallbackTimeInfo *time_info,
        PaStreamCallbackFlags flags, void *arg)
{
    struct audio *audio = arg;

    if (audio->should_stop) {
        sf_seek(audio->current_file, 0, SEEK_SET);
        audio->should_stop = false;
    } else {
        sf_read_short(audio->current_file, output, frames_per_buffer * 2);
    }

    return paContinue;
}

void audio_init(struct audio *audio)
{
    audio->current_stream = NULL;
    audio->should_stop = false;

    if (paNoError != (err = Pa_Initialize())) {
        FPRINTF(stderr, "portaudio Pa_Initialize() error: %s\n", Pa_GetErrorText(err));
        exit(EXIT_FAILURE);
    }

    PaStreamParameters out_param;
    out_param.device = Pa_GetDefaultOutputDevice();
    if (out_param.device == paNoDevice) {
        FPRINTF(stderr, "portaudio error: no audio device found\n");
        exit(EXIT_FAILURE);
    }

    SF_INFO sf_info;
    for (int i = 0; i < NUM__AUDIO_STREAM; ++i) {
        audio->files[i] = sf_open(audio_stream_paths[i], SFM_READ, &sf_info);
        if (!audio->files[i]) {
            FPRINTF(stderr, "libsndfile sf_open() error: could not open %s\n",
                    audio_stream_paths[i]);
            exit(EXIT_FAILURE);
        }

        out_param.channelCount = sf_info.channels;
        out_param.sampleFormat = paInt16;
        out_param.suggestedLatency = Pa_GetDeviceInfo(out_param.device)->defaultLowOutputLatency;
        out_param.hostApiSpecificStreamInfo = NULL;

        audio->current_file = audio->files[i];
        if (paNoError != (err = Pa_OpenStream(&audio->streams[i], NULL, &out_param,
                        sf_info.samplerate, paFramesPerBufferUnspecified,
                        paClipOff, _audio_sndfile_cb, audio))) {
            FPRINTF(stderr, "portaudio Pa_OpenStream() error: %s\n", Pa_GetErrorText(err));
            exit(EXIT_FAILURE);
        }
    }
}

void audio_stop(struct audio *audio)
{
    if (!audio->current_stream)
        return;

    if (paNoError != (err = Pa_AbortStream(audio->current_stream)))
        FPRINTF(stderr, "portaudio Pa_StopStream() error: %s\n", Pa_GetErrorText(err));

    audio->current_stream = NULL;
    audio->should_stop = true;
}

void audio_play(struct audio *audio, enum audio_stream stream)
{
    audio_stop(audio);

    if (paNoError != (err = Pa_StartStream(audio->streams[stream])))
        FPRINTF(stderr, "portaudio Pa_StartStream() error: %s\n", Pa_GetErrorText(err));

    audio->current_stream = audio->streams[stream];
}

void audio_destroy(struct audio *audio)
{
    if (paNoError != (err = Pa_CloseStream(audio->streams[AUDIO_STREAM__BATTLE]))) {
        FPRINTF(stderr, "portaudio Pa_CloseStream() error: %s\n", Pa_GetErrorText(err));
        exit(EXIT_FAILURE);
    }

    // TODO save file handles... sf_close(file);

    Pa_Terminate();
}
