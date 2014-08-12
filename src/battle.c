#include "battle.h"

struct battle *battle_new(struct player *player)
{
    struct battle *battle = MALLOC(sizeof(*battle));

    battle->player = player;
    battle->monster_bot = player->monsters[0];
    // TODO free
    battle->monster_top = monster_new(true);

    return battle;
}

void battle_free(struct battle *battle)
{
    free(battle);
}
