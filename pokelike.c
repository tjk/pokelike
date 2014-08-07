#include <curses.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define COLOR_GRASS 1

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

#define NUM_FLICKER_FRAMES 8

struct animation {
    int frame;
    int num_frames;
    animation_render_func animation_render_func;
};

void animation_curtain_render(int frame, int num_frames, int h, int w)
{
    if (frame <= num_frames || frame % 2 == 0) {
        for (int y = 0; y < (int)(h * (float)frame / num_frames); ++y)
            for (int x = 0; x < w; ++x)
                mvaddch(y, x, ACS_CKBOARD);
    }
}

static struct animation animation_curtain = {
    .frame = 0,
    .num_frames = 8,
    .animation_render_func = animation_curtain_render,
};

struct game {
    struct player *player;
    struct chunk *chunk; // TODO map? (lots of chunks)
    struct animation animation;
    struct battle *battle;
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
            // TODO instead of 20%, make it based on the patch of grass...
            if (rand() % 100 < 5) {
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

int main(void)
{
    signal(SIGINT, _finish);

    srand(time(NULL));

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

    endwin();
    return EXIT_SUCCESS;
}
