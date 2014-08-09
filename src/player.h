#pragma once

#include "common.h"

struct player {
    int y;
    int x;
};

struct player *player_new();

void player_free(struct player *player);
