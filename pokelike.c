#include <curses.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define COLOR_GRASS 1

struct player {
    int y;
    int x;
};

struct map {
    int h;
    int w;
    char *tiles;
};

struct game {
    struct player *player;
    struct map *map;
    bool running;
} game;

static void _finish(int sig)
{
    game.running = false;
}

static void game_handle_input(struct game *game, int c)
{
    switch (c) {
    case KEY_UP:
    case 'w':
        --game->player->y;
        break;
    case KEY_DOWN:
    case 's':
        ++game->player->y;
        break;
    case KEY_RIGHT:
    case 'd':
        ++game->player->x;
        break;
    case KEY_LEFT:
    case 'a':
        --game->player->x;
        break;
    }
}

static void game_tick(struct game *game)
{
    // TODO
}

static void game_render(struct game *game)
{
    erase();

    // TODO switch on game state, walking around vs. in battle

    // draw border
    box(stdscr, 0, 0);

    for (int y = 0; y < game->map->h; ++y) {
        for (int x = 0; x < game->map->w; ++x) {
            switch (game->map->tiles[y * (game->map->w + 1) + x]) {
            case 'v':
                attron(COLOR_PAIR(COLOR_GRASS));
                mvaddch(y, x, 'v');
                attroff(COLOR_PAIR(COLOR_GRASS));
                break;
            }
        }
    }

    mvaddch(game->player->y, game->player->x, '@');

    refresh();
}

int main(void)
{
    signal(SIGINT, _finish);

    initscr();
    keypad(stdscr, true);
    nonl();
    cbreak(); // TODO use raw() and handle CTRL inputs manually (no signal)
    noecho();
    curs_set(false);
    nodelay(stdscr, true);

    if (has_colors()) {
        start_color();

        init_pair(COLOR_GRASS, COLOR_WHITE, COLOR_GREEN);
    }

    struct player player = { .y = 10, .x = 10 };
    game.player = &player;

    struct map map;
    // TODO ensure each "row" is same length (probably use 2D array)
    map.tiles = "                                                                    \n"
                "      vvvv                         vvvvvvvvvvvvvvvvvvvvvvvvvvvv     \n"
                "      vvvv                         vvvvvvvvvvvvvvvvvvvvvvvvvvvv     \n"
                "      vvvv                         vvvvvvvvvvvvvvvvvvvvvvvvvvvv     \n"
                "      vvvv                         vvvvvvvvvvvvvvvvvvvvvvvvvvvv     \n"
                "      vvvv                                                          \n"
                "      vvvv                                                          \n"
                "      vvvv                                                          \n";
    char *first_newline = strchr(map.tiles, '\n');
    map.w = first_newline ? first_newline - map.tiles : strlen(map.tiles);
    int rows = 0;
    while (first_newline) {
        first_newline = strchr(first_newline + 1, '\n');
        ++rows;
    }
    map.h = rows ? rows : 1;
    game.map = &map;

    game.running = true;

    while (game.running) {
        int c = getch();
        game_handle_input(&game, c);

        game_tick(&game);
        game_render(&game);

        usleep(100000);
    }

    endwin();
    return EXIT_SUCCESS;
}
