#pragma once

#include "common.h"

#define MAP_CHUNK_WIDTH 80
#define MAP_CHUNK_HEIGHT 25

struct map_chunk {
    char tiles[MAP_CHUNK_HEIGHT][MAP_CHUNK_WIDTH];
};

void map_chunk_init(struct map_chunk *chunk);
