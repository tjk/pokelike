#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "audio.h"
#include "common.h"
#include "game.h"
#include "map.h"

static void _finish(int sig)
{
    exit(EXIT_SUCCESS);
}

static void _exit_cb(void)
{
    // clean up ncurses so terminal is not left in funky state
    endwin();
}

int main(void)
{
    // initialize stdlib things
    signal(SIGINT, _finish);
    srand(time(NULL));
    atexit(_exit_cb);

    // initialize ncurses
    initscr();
    keypad(stdscr, true);
    nonl();
    cbreak(); // TODO use raw() and handle CTRL inputs manually (no signal)
    noecho();
    curs_set(false);
    nodelay(stdscr, true);
    // TODO support no color mode (just investigate if attron works)
    if (!has_colors()) {
        FPRINTF(stderr, "ncurses colors not supported\n");
        exit(EXIT_FAILURE);
    }
    start_color();
    init_pair(COLOR__GRASS, COLOR_WHITE, COLOR_GREEN);
    init_pair(COLOR__HP_GOOD, COLOR_GREEN, COLOR_GREEN);
    init_pair(COLOR__HP_WARNING, COLOR_YELLOW, COLOR_YELLOW);
    init_pair(COLOR__HP_DANGER, COLOR_RED, COLOR_RED);

    struct game game;
    audio_init(&game.audio);
    game_init(&game);

    // TODO clean this up
    struct map_chunk chunk;
    map_chunk_init(&chunk);
    game.chunk = &chunk;

    // TODO improve game loop
    // - "rendering" and update tick do not need to have same rate
    // - fix input triggering after letting go, etc.
    for (;;) {
        int c = getch();
        game_handle_input(&game, c);

        game_tick(&game);

        game_render(&game);
        ++game.frame;

        usleep(100000);
    }

    // clean up audio
    audio_destroy(&game.audio);

    return EXIT_SUCCESS;
}
