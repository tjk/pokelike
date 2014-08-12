#pragma once

#include "common.h"
#include "monster.h"

#define PLAYER_MONSTER_SIZE 6

struct player {
    int y;
    int x;
    struct monster *monsters[PLAYER_MONSTER_SIZE];
};

struct player *player_new();

void player_free(struct player *player);
