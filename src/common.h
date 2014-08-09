#pragma once

#include <stdlib.h>

#include <curses.h>

#define COLOR_GRASS 1

#define TILE_EMPTY ' '
#define TILE_GRASS 'v'

#define FPRINTF(fd, ...) do { \
    def_prog_mode(); \
    endwin(); \
    fprintf(fd, __VA_ARGS__); \
    refresh(); \
} while (0);
