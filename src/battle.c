#include "battle.h"

struct battle *battle_new()
{
    struct battle *battle = malloc(sizeof(*battle));

    // TODO

    return battle;
}

void battle_free(struct battle *battle)
{
    free(battle);
}
