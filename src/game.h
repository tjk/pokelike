#pragma once

#include "common.h"

#include "animation.h"
#include "audio.h"
#include "player.h"

enum game_state {
    GAME_STATE__EXPLORE = 0,
    GAME_STATE__BATTLE_PRE_ANIMATION,
    GAME_STATE__BATTLE,
};

struct game {
    struct player *player;
    struct map_chunk *chunk; // TODO map? (lots of chunks)
    struct audio audio;
    struct animation *animation;
    struct battle *battle;
    enum game_state state;
    bool running;
    char debug[1024];
};

void game_init(struct game *game);
void game_init_audio(struct game *game);

void game_tick(struct game *game);

void game_render(struct game *game);

void game_handle_input(struct game *game, int c);
