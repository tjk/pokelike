#pragma once

#include "common.h"
#include "player.h"

typedef enum {
    BATTLE_STATE__MAIN_MENU = 0,
    BATTLE_STATE__FIGHT, // TODO rename to SELECT_MOVE?
    BATTLE_STATE__FIGHTING,
    // BATTLE_STATE__TEAM
    // BATTLE_STATE__PACK
} battle_state_t;

struct battle {
    battle_state_t state;
    struct player *player;
    struct monster *monster_bot;
    struct monster_move *move_bot;
    struct monster *monster_top;
    struct monster_move *move_top;
};

struct battle *battle_new(struct player *player);

void battle_free(struct battle *battle);
