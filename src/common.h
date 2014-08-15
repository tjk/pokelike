#pragma once

#include <stdlib.h>

#include <curses.h>

typedef enum {
    COLOR__GRASS = 1,
    COLOR__HP_GOOD,
    COLOR__HP_WARNING,
    COLOR__HP_DANGER,
    COLOR__FG_BG_SWAPPED,
} color_t;

#define TILE_EMPTY ' '
#define TILE_GRASS 'v'

#define FPRINTF(fd, ...) do { \
    def_prog_mode(); \
    endwin(); \
    fprintf(fd, __VA_ARGS__); \
    refresh(); \
} while (0);

#define MALLOC(size) _malloc(size)
static inline void *_malloc(int size)
{
    void *ptr = malloc(size);
    if (!ptr) {
        FPRINTF(stderr, "out of memory\n");
        exit(EXIT_FAILURE);
    }

    return ptr;
}

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

#define ARRAY_SIZE(x) (sizeof(x)/sizeof((x)[0]))
