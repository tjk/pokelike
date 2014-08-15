#include "battle.h"

struct battle *battle_new(struct player *player)
{
    struct battle *battle = MALLOC(sizeof(*battle));

    battle->player = player;
    battle->monster_bot = player->monsters[0];
    battle->move_bot = NULL;
    // TODO free
    battle->monster_top = monster_new(true);
    battle->move_top = NULL;
    battle->state = BATTLE_STATE__MAIN_MENU;

    return battle;
}

void battle_free(struct battle *battle)
{
    free(battle);
}
