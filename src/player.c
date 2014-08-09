#include "player.h"

#include <stdlib.h>

struct player *player_new(int y, int x)
{
    struct player *player = malloc(sizeof(*player));

    player->y = y;
    player->x = x;

    return player;
}

void player_free(struct player *player)
{
    free(player);
}
