#include "monster.h"

#include <string.h>

// TODO types should match up with monster_type_t
static struct monster_type monster_types[] = {
    {
        .name = "Bug",
        .weak_to = { MONSTER_TYPE__FIGHTING, MONSTER_TYPE__FLYING, MONSTER_TYPE__GHOST, MONSTER_TYPE__FIRE },
        .strong_against = { MONSTER_TYPE__POISON, MONSTER_TYPE__GRASS, MONSTER_TYPE__PSYCHIC },
    }
};

static struct monster_move monster_moves[] = {
    {
        .name = "Tackle",
        .description = "Most basic, weak, normal attack.",
        .type = MONSTER_TYPE__NORMAL,
        .base_dmg = 5,
    }
};

struct monster_species *monster_species_new()
{
    struct monster_species *monster_species = MALLOC(sizeof(*monster_species));

    snprintf(monster_species->name, sizeof(monster_species->name), "Pikachu"); // TODO rename
    memset(monster_species->types, 0,
            sizeof(monster_species->types)/sizeof(*monster_species->types));
    monster_species->types[0] = MONSTER_TYPE__ELECTRIC;
    for (int y = 0; y < MONSTER_SPECIES_BITMAP_H; ++y) {
        for (int x = 0; x < MONSTER_SPECIES_BITMAP_W / 2; ++x) {
            int on = rand() % 2;
            monster_species->bitmap[y][x] = on;
            monster_species->bitmap[y][MONSTER_SPECIES_BITMAP_W - x - 1] = on;
        }
        if (MONSTER_SPECIES_BITMAP_W % 2 == 1)
            monster_species->bitmap[y][MONSTER_SPECIES_BITMAP_W / 2] = rand() % 2;
    }
    int i;
    do {
         i = rand() % COLORS;
         monster_species->color = i;
    } while (i >= NUM__COLOR);

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
    for (int i = 0; i < sizeof(monster->moves)/sizeof(*monster->moves); ++i)
        monster->moves[i] = NULL;
    monster->moves[0] = &monster_moves[0]; // TODO use MONSTER_MOVE__TACKLE (instead of 0)
    monster->num_moves = 1;

    return monster;
}
