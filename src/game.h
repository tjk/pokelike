#pragma once

#include "common.h"

#include "animation.h"
#include "audio.h"
#include "player.h"

#define GAME_DEBUG_SIZE 1024

typedef enum {
    GAME_STATE__EXPLORE = 0,
    GAME_STATE__PRE_BATTLE,
    GAME_STATE__BATTLE,
} game_state_t;

typedef enum {
    CURSOR__FIGHT = 0,
    CURSOR__TEAM,
    CURSOR__PACK,
    CURSOR__RUN,

    CURSOR__FIGHT_MOVE,
    CURSOR__FIGHT_MOVE_1 = CURSOR__FIGHT_MOVE,
    CURSOR__FIGHT_MOVE_2,
    CURSOR__FIGHT_MOVE_3,
    CURSOR__FIGHT_MOVE_4,
} cursor_t;

typedef enum {
    WINDOW__BATTLE_PRIMARY = 0,
    WINDOW__BATTLE_SECONDARY,
    // TODO menus - windows? panels?

    NUM__WINDOW,
} window_t;

struct game {
    struct player *player;
    struct map_chunk *chunk; // TODO map? (lots of chunks)
    struct audio audio;
    struct animation *animation;
    struct battle *battle;
    WINDOW *windows[NUM__WINDOW]; // TODO de-couple ncurses?
    game_state_t state;
    bool running;
    char debug[GAME_DEBUG_SIZE];
    unsigned int frame; // TODO obvious rollover issue
    unsigned int saved_frame;
    cursor_t cursor;
};

void game_init(struct game *game);
void game_init_audio(struct game *game);

void game_set_state(struct game *game, game_state_t state);

void game_tick(struct game *game);

void game_render(struct game *game);

void game_handle_input(struct game *game, int c);
