#include "game.h"

#include <curses.h>

#include <unistd.h>

#include "audio.h"
#include "battle.h"
#include "map.h"

// TODO move this
static struct animation animations[] = {
    { .frame = 0, .num_frames = 12, .render_func = animation_curtain_render },
    { .frame = 0, .num_frames = 12, .render_func = animation_inward_render },
    { .frame = 0, .num_frames = 12, .render_func = animation_snake_render },
};

void game_init(struct game *game)
{
    game->player = player_new(10, 10);
}

void game_tick(struct game *game)
{
    switch (game->state) {
    case GAME_STATE__EXPLORE:
        if (game->chunk->tiles[game->player->y][game->player->x] == TILE_GRASS) {
            // TODO instead of 5%, make it based on the patch of grass...
            if (rand() % 100 < 5) {
                audio_play(&game->audio, AUDIO_STREAM__BATTLE);
                // TODO instead of sleeping, need to either:
                // 1) start audio immediately, or
                // 2) run the following code when the playback callback is first triggered
                sleep(1);
                game->state = GAME_STATE__BATTLE_PRE_ANIMATION;
                int r = rand() % (sizeof(animations)/sizeof(animations[0]));
                game->animation = &animations[r];
                game->animation->frame = 0;
            } else {
                snprintf(game->debug, sizeof(game->debug), "Player standing on grass.");
            }
        } else {
            snprintf(game->debug, sizeof(game->debug), "Exploring the map.");
        }
        break;
    case GAME_STATE__BATTLE_PRE_ANIMATION:
        snprintf(game->debug, sizeof(game->debug), "Playing animation! [%d/%d]", game->animation->frame, game->animation->num_frames);
        ++game->animation->frame;
        if (game->animation->frame == game->animation->num_frames + NUM_FLICKER_FRAMES) {
            game->battle = battle_new();
            game->state = GAME_STATE__BATTLE;
        }
        break;
    case GAME_STATE__BATTLE:
        snprintf(game->debug, sizeof(game->debug), "It's battle time.");
        break;
    }
}

void game_render(struct game *game)
{
    erase();

    int h, w;
    getmaxyx(stdscr, h, w);

    switch (game->state) {
    case GAME_STATE__EXPLORE:
        box(stdscr, 0, 0);
        for (int y = 0; y < MAP_CHUNK_HEIGHT; ++y) {
            for (int x = 0; x < MAP_CHUNK_WIDTH; ++x) {
                switch (game->chunk->tiles[y][x]) {
                case TILE_GRASS:
                    attron(COLOR_PAIR(COLOR_GRASS));
                    mvaddch(y, x, TILE_GRASS);
                    attroff(COLOR_PAIR(COLOR_GRASS));
                    break;
                }
            }
        }
        mvaddch(game->player->y, game->player->x, '@');
        break;
    case GAME_STATE__BATTLE_PRE_ANIMATION:
        box(stdscr, 0, 0);
        game->animation->render_func(game->animation, h, w);
        break;
    case GAME_STATE__BATTLE:;
        // TODO
        break;
    }

    mvprintw(0, 0, game->debug);

    refresh();
}

void game_handle_input(struct game *game, int c)
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

