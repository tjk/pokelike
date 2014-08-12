#pragma once

#include "common.h"
#include "player.h"

struct battle {
    struct player *player;
    struct monster *monster_bot;
    struct monster *monster_top;
};

struct battle *battle_new(struct player *player);

void battle_free(struct battle *battle);
