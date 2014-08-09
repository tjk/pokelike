#include "map.h"

#include <stdlib.h>

void map_chunk_init(struct map_chunk *chunk)
{
    // TODO pick a couple points to make grass patches at
    for (int y = 0; y < MAP_CHUNK_HEIGHT; ++y)
        for (int x = 0; x < MAP_CHUNK_WIDTH; ++x)
            chunk->tiles[y][x] = TILE_EMPTY;

    int num_patches = rand() % 16;
    for (int i = 0; i < num_patches; ++i) {
        int patch_y = rand() % MAP_CHUNK_HEIGHT;
        int patch_x = rand() % MAP_CHUNK_WIDTH;
        int patch_h = rand() % 5;
        int patch_w = rand() % 20;
        for (int y = patch_y; y < patch_y + patch_h && y < MAP_CHUNK_HEIGHT; ++y)
            for (int x = patch_x; x < patch_x + patch_w && x < MAP_CHUNK_WIDTH; ++x)
                chunk->tiles[y][x] = TILE_GRASS;
    }
}
