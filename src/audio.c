#include "audio.h"

// must match with enum audio_stream
static char *audio_stream_paths[] = {
    "./assets/battle.wav",
};

static int _audio_sndfile_cb(const void *input, void *output, unsigned long frames_per_buffer,
        const PaStreamCallbackTimeInfo *time_info,
        PaStreamCallbackFlags flags, void *arg)
{
    SNDFILE *file = arg;

    sf_read_short(file, output, frames_per_buffer * 2);

    return paContinue;
}

void audio_init(struct audio *audio)
{
    if ((err = Pa_Initialize())) {
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
    SNDFILE *file;
    for (int i = 0; i < NUM__AUDIO_STREAM; ++i) {
        file = sf_open(audio_stream_paths[i], SFM_READ, &sf_info);
        if (!file) {
            FPRINTF(stderr, "libsndfile sf_open() error: could not open %s\n",
                    audio_stream_paths[i]);
            exit(EXIT_FAILURE);
        }

        out_param.channelCount = sf_info.channels;
        out_param.sampleFormat = paInt16;
        out_param.suggestedLatency = Pa_GetDeviceInfo(out_param.device)->defaultLowOutputLatency;
        out_param.hostApiSpecificStreamInfo = NULL;

        if ((err = Pa_OpenStream(&audio->streams[i], NULL, &out_param,
                        sf_info.samplerate, paFramesPerBufferUnspecified,
                        paClipOff, _audio_sndfile_cb, file))) {
            FPRINTF(stderr, "portaudio Pa_OpenStream() error: %s\n", Pa_GetErrorText(err));
            exit(EXIT_FAILURE);
        }
    }
}

void audio_play(struct audio *audio, enum audio_stream stream)
{
    for (int i = 0; i < NUM__AUDIO_STREAM; ++i) {
        if ((err = Pa_StartStream(audio->streams[i])))
            FPRINTF(stderr, "portaudio Pa_StartStream() error: %s\n", Pa_GetErrorText(err));
    }
}

void audio_destroy(struct audio *audio)
{
    if ((err = Pa_CloseStream(audio->streams[AUDIO_STREAM__BATTLE]))) {
        FPRINTF(stderr, "portaudio Pa_CloseStream() error: %s\n", Pa_GetErrorText(err));
        exit(EXIT_FAILURE);
    }
    // TODO save file handles... sf_close(file);
    Pa_Terminate();
}
