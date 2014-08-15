#pragma once

#include "common.h"

#define MONSTER_SPECIES_NAME_SIZE 64

typedef enum {
    MONSTER_TYPE__BUG = 0,
    MONSTER_TYPE__DRAGON,
    MONSTER_TYPE__ICE,
    MONSTER_TYPE__FIGHTING,
    MONSTER_TYPE__FIRE,
    MONSTER_TYPE__FLYING,
    MONSTER_TYPE__GRASS,
    MONSTER_TYPE__GHOST,
    MONSTER_TYPE__GROUND,
    MONSTER_TYPE__ELECTRIC,
    MONSTER_TYPE__NORMAL,
    MONSTER_TYPE__POISON,
    MONSTER_TYPE__PSYCHIC,
    MONSTER_TYPE__ROCK,
    MONSTER_TYPE__WATER,

    NUM__MONSTER_TYPE,
} monster_type_t;

// TODO replace magic number for max num of weak / strong types
struct monster_type {
    const char *name;
    monster_type_t weak_to[5];
    monster_type_t strong_against[5];
};

#define MONSTER_SPECIES_BITMAP_H 5
#define MONSTER_SPECIES_BITMAP_W 20

// TODO replace magic number for max num types
struct monster_species {
    char name[MONSTER_SPECIES_NAME_SIZE]; // TODO move this to monster if allow rename
    monster_type_t types[3];
    bool bitmap[MONSTER_SPECIES_BITMAP_H][MONSTER_SPECIES_BITMAP_W];
    int color;
    // TODO base stats that are randomized slightly for different instances
};

struct monster_move {
    const char *name;
    const char *description;
    monster_type_t type;
    int base_dmg;
    // TODO add multipliers, chance of effects, etc.
};

#define MONSTER_MOVES_SIZE 4

struct monster {
    struct monster_species *species;
    int num_moves;
    struct monster_move *moves[MONSTER_MOVES_SIZE]; // TODO add PP (or even better? MANA)
    int level;
    int hp;
    int hp_total;
    // TODO for attributes, ie. atk: use atk and cur_atk (for stat boosts)
};

struct monster_species *monster_species_new();

struct monster *monster_new(bool is_damaged);
