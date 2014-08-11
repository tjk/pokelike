#pragma once

#include <stdlib.h>

#include <curses.h>

enum color {
    COLOR__GRASS = 1,
    COLOR__HP_GOOD,
};

#define TILE_EMPTY ' '
#define TILE_GRASS 'v'

#define FPRINTF(fd, ...) do { \
    def_prog_mode(); \
    endwin(); \
    fprintf(fd, __VA_ARGS__); \
    refresh(); \
} while (0);
