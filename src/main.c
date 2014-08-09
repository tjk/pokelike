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
    init_pair(COLOR_GRASS, COLOR_WHITE, COLOR_GREEN);

    struct game game;
    game_init(&game);
    audio_init(&game.audio);

    // TODO clean up this
    struct map_chunk chunk;
    map_chunk_init(&chunk);
    game.chunk = &chunk;

    for (;;) {
        int c = getch();
        game_handle_input(&game, c);

        game_tick(&game);
        game_render(&game);

        usleep(100000);
    }

    // clean up audio
    audio_destroy(&game.audio);

    return EXIT_SUCCESS;
}
