#include "player.h"

#include <stdlib.h>
#include <string.h>

struct player *player_new(int y, int x)
{
    struct player *player = malloc(sizeof(*player));

    player->y = y;
    player->x = x;

    memset(player->monsters, 0, sizeof(player->monsters)/sizeof(*player->monsters));
    // TODO free
    player->monsters[0] = monster_new(false);

    return player;
}

void player_free(struct player *player)
{
    for (int i = 0; i < PLAYER_MONSTER_SIZE; ++i)
        free(player->monsters[i]);
    free(player);
}
