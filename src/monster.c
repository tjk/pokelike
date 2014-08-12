#include "monster.h"

#include <string.h>

static struct monster_type monster_types[1] = {
    {
        .name = "Bug",
        .weak_to = { MONSTER_TYPE__FIGHTING, MONSTER_TYPE__FLYING, MONSTER_TYPE__GHOST, MONSTER_TYPE__FIRE },
        .strong_against = { MONSTER_TYPE__POISON, MONSTER_TYPE__GRASS, MONSTER_TYPE__PSYCHIC },
    }
};

struct monster_species *monster_species_new()
{
    struct monster_species *monster_species = MALLOC(sizeof(*monster_species));

    snprintf(monster_species->name, sizeof(monster_species->name), "Pikachu"); // TODO rename
    memset(monster_species->types, 0,
            sizeof(monster_species->types)/sizeof(*monster_species->types));
    monster_species->types[0] = MONSTER_TYPE__ELECTRIC;

    return monster_species;
}

// can be damaged up to 25% hp
struct monster *monster_new(bool is_damaged)
{
    struct monster *monster = MALLOC(sizeof(*monster));

    // TODO free (these should be initialized at beginning and free'ed at way end)
    monster->species = monster_species_new();
    monster->level = rand() % 10;
    monster->hp_total = (3 + rand() % 5) * monster->level;
    monster->hp = monster->hp_total;
    if (is_damaged) {
        int max_damaged_by = MAX(monster->hp_total / 4, 1);
        monster->hp -= rand() % max_damaged_by;
    }

    return monster;
}
