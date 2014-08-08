#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <curses.h>
#include <portaudio.h>
#include <sndfile.h>

#define FPRINTF(fd, ...) do { \
    def_prog_mode(); \
    endwin(); \
    fprintf(fd, __VA_ARGS__); \
    refresh(); \
} while (0);

#define COLOR_GRASS 1

PaError err;

struct player {
    int y;
    int x;
};

#define CHUNK_WIDTH 80
#define CHUNK_HEIGHT 25

#define TILE_EMPTY ' '
#define TILE_GRASS 'v'

struct chunk {
    char tiles[CHUNK_HEIGHT][CHUNK_WIDTH];
};

enum state {
    STATE_EXPLORE = 0,
    STATE_BATTLE_PRE_ANIMATION,
    STATE_BATTLE,
};

typedef void (*animation_render_func)(int frame, int num_frames, int h, int w);

#define NUM_FLICKER_FRAMES 12

struct animation {
    int frame;
    int num_frames;
    animation_render_func animation_render_func;
};

void animation_curtain_render(int frame, int num_frames, int h, int w)
{
    if (frame <= num_frames || frame % 3 == 0) {
        for (int y = 0; y < (int)(h * (float)frame / num_frames); ++y)
            for (int x = 0; x < w; ++x)
                mvaddch(y, x, ACS_CKBOARD);
    }
}

static struct animation animation_curtain = {
    .frame = 0,
    .num_frames = 12,
    .animation_render_func = animation_curtain_render,
};

enum stream {
    STREAM_BATTLE = 0,

    NUM_STREAMS,
};

struct game {
    struct player *player;
    struct chunk *chunk; // TODO map? (lots of chunks)
    struct animation animation;
    struct battle *battle;
    PaStream *streams[NUM_STREAMS];
    enum state state;
    bool running;
    char debug[1024];
} game;

struct battle {
    // TODO stuff!
};

static void _finish(int sig)
{
    game.running = false;
}

static void game_handle_input(struct game *game, int c)
{
    switch (c) {
    case KEY_UP:
    case 'w':
        --game->player->y;
        break;
    case KEY_DOWN:
    case 's':
        ++game->player->y;
        break;
    case KEY_RIGHT:
    case 'd':
        ++game->player->x;
        break;
    case KEY_LEFT:
    case 'a':
        --game->player->x;
        break;
    }
}

static struct battle *battle_new()
{
    struct battle *battle = malloc(sizeof(*battle));

    return battle;
}

static void game_tick(struct game *game)
{
    switch (game->state) {
    case STATE_EXPLORE:
        if (game->chunk->tiles[game->player->y][game->player->x] == TILE_GRASS) {
            // TODO instead of 5%, make it based on the patch of grass...
            if (rand() % 100 < 5) {
                if ((err = Pa_StartStream(game->streams[STREAM_BATTLE])))
                    FPRINTF(stderr, "portaudio Pa_StartStream() error: %s\n", Pa_GetErrorText(err));
                // TODO instead of sleeping, need to either:
                // 1) start audio immediately, or
                // 2) run the following code when the playback callback is first triggered
                sleep(1);
                game->state = STATE_BATTLE_PRE_ANIMATION;
                game->animation = animation_curtain;
                game->animation.frame = 0;
            } else {
                snprintf(game->debug, sizeof(game->debug), "Player standing on grass.");
            }
        } else {
            snprintf(game->debug, sizeof(game->debug), "Exploring the map.");
        }
        break;
    case STATE_BATTLE_PRE_ANIMATION:
        snprintf(game->debug, sizeof(game->debug), "Playing animation! [%d/%d]", game->animation.frame, game->animation.num_frames);
        ++game->animation.frame;
        if (game->animation.frame == game->animation.num_frames + NUM_FLICKER_FRAMES) {
            game->battle = battle_new();
            game->state = STATE_BATTLE;
        }
        break;
    case STATE_BATTLE:
        snprintf(game->debug, sizeof(game->debug), "It's battle time.");
        break;
    }
}

static void game_render(struct game *game)
{
    erase();

    int h, w;
    getmaxyx(stdscr, h, w);

    // draw border
    box(stdscr, 0, 0);

    switch (game->state) {
    case STATE_EXPLORE:
        for (int y = 0; y < CHUNK_HEIGHT; ++y) {
            for (int x = 0; x < CHUNK_WIDTH; ++x) {
                switch (game->chunk->tiles[y][x]) {
                case TILE_GRASS:
                    attron(COLOR_PAIR(COLOR_GRASS));
                    mvaddch(y, x, TILE_GRASS);
                    attroff(COLOR_PAIR(COLOR_GRASS));
                    break;
                }
            }
        }
        mvaddch(game->player->y, game->player->x, '@');
        break;
    case STATE_BATTLE_PRE_ANIMATION:
        game->animation.animation_render_func(game->animation.frame, game->animation.num_frames, h, w);
        break;
    case STATE_BATTLE:
        // TODO
        break;
    }

    mvprintw(0, 0, game->debug);

    refresh();
}

static void chunk_init(struct chunk *chunk)
{
    // TODO pick a couple points to make grass patches at
    for (int y = 0; y < CHUNK_HEIGHT; ++y)
        for (int x = 0; x < CHUNK_WIDTH; ++x)
            chunk->tiles[y][x] = TILE_EMPTY;

    int num_patches = rand() % 16;
    for (int i = 0; i < num_patches; ++i) {
        int patch_y = rand() % CHUNK_HEIGHT;
        int patch_x = rand() % CHUNK_WIDTH;
        int patch_h = rand() % 5;
        int patch_w = rand() % 20;
        for (int y = patch_y; y < patch_y + patch_h && y < CHUNK_HEIGHT; ++y)
            for (int x = patch_x; x < patch_x + patch_w && x < CHUNK_WIDTH; ++x)
                chunk->tiles[y][x] = TILE_GRASS;
    }
}

static int sndfile_cb(const void *input, void *output, unsigned long frames_per_buffer,
        const PaStreamCallbackTimeInfo *time_info,
        PaStreamCallbackFlags flags, void *arg)
{
    SNDFILE *file = arg;

    sf_read_short(file, output, frames_per_buffer * 2);

    return paContinue;
}

static void _exit_cb(void)
{
    // clean up ncurses so terminal is not left in funky state
    endwin();
}

int main(void)
{
    // initialize stdlib things
    signal(SIGINT, _finish);
    srand(time(NULL));
    atexit(_exit_cb);

    // initialize ncurses
    initscr();
    keypad(stdscr, true);
    nonl();
    cbreak(); // TODO use raw() and handle CTRL inputs manually (no signal)
    noecho();
    curs_set(false);
    nodelay(stdscr, true);
    if (has_colors()) {
        start_color();
        init_pair(COLOR_GRASS, COLOR_WHITE, COLOR_GREEN);
    }

    // initialize audio
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
    SNDFILE *file = sf_open("./assets/battle.wav", SFM_READ, &sf_info);
    if (!file) {
        FPRINTF(stderr, "libsndfile sf_open() error: could not open assets/battle.wav\n");
        exit(EXIT_FAILURE);
    }
    FPRINTF(stdout, "%d frames %d samplerate %d channels\n",
            (int)sf_info.frames, sf_info.samplerate, sf_info.channels);

    out_param.channelCount = sf_info.channels;
    out_param.sampleFormat = paInt16;
    out_param.suggestedLatency = Pa_GetDeviceInfo(out_param.device)->defaultLowOutputLatency;
    out_param.hostApiSpecificStreamInfo = NULL;

    // initialize game
    if ((err = Pa_OpenStream(&game.streams[STREAM_BATTLE], NULL, &out_param,
                    sf_info.samplerate, paFramesPerBufferUnspecified,
                    paClipOff, sndfile_cb, file))) {
        FPRINTF(stderr, "portaudio Pa_OpenStream() error: %s\n", Pa_GetErrorText(err));
        exit(EXIT_FAILURE);
    }

    struct player player = { .y = 10, .x = 10 };
    game.player = &player;

    struct chunk chunk;
    chunk_init(&chunk);
    game.chunk = &chunk;

    game.running = true;

    while (game.running) {
        int c = getch();
        game_handle_input(&game, c);

        game_tick(&game);
        game_render(&game);

        usleep(100000);
    }

    // clean up audio
    if ((err = Pa_CloseStream(game.streams[STREAM_BATTLE]))) {
        FPRINTF(stderr, "portaudio Pa_CloseStream() error: %s\n", Pa_GetErrorText(err));
        exit(EXIT_FAILURE);
    }
    sf_close(file);
    Pa_Terminate();

    return EXIT_SUCCESS;
}
