#include "game.h"

#include <curses.h>

#include <unistd.h>

#include "audio.h"
#include "battle.h"
#include "map.h"

// TODO move this + these could just be func pointers since num_frames is constant...
static struct animation animations[] = {
    { .num_frames = 12, .render_func = animation_curtain_render },
    { .num_frames = 12, .render_func = animation_inward_render },
    { .num_frames = 12, .render_func = animation_snake_render },
};

void game_init(struct game *game)
{
    game->player = player_new(10, 10);
    game->frame = 0;
    game_set_state(game, GAME_STATE__EXPLORE);

    // TODO delwin these in game_{cleanup,destroy,free}
    for (int i = 0; i < NUM__WINDOW; ++i) {
        game->windows[i] = newwin(0, 0, 0, 0);
    }
}

void game_tick(struct game *game)
{
    switch (game->state) {
    case GAME_STATE__EXPLORE:
        if (game->chunk->tiles[game->player->y][game->player->x] == TILE_GRASS) {
            // TODO instead of 5%, make it based on the patch of grass...
            if (rand() % 100 < 5) {
                game_set_state(game, GAME_STATE__PRE_BATTLE);
            } else {
                snprintf(game->debug, sizeof(game->debug), "Player standing on grass.");
            }
        } else {
            snprintf(game->debug, sizeof(game->debug), "Exploring the map.");
        }
        break;
    case GAME_STATE__PRE_BATTLE:;
        int frame = game->frame - game->saved_frame;
        snprintf(game->debug, sizeof(game->debug), "Playing animation! [%d/%d]",
                frame, game->animation->num_frames);
        if (frame == game->animation->num_frames + NUM_FLICKER_FRAMES)
            game_set_state(game, GAME_STATE__BATTLE);
        break;
    case GAME_STATE__BATTLE:
        break;
    }
}

void game_set_state(struct game *game, game_state_t state)
{
    switch (state) {
    case GAME_STATE__EXPLORE:
        audio_play(&game->audio, AUDIO_STREAM__EXPLORE);
        break;
    case GAME_STATE__PRE_BATTLE:;
        // select a random animation
        int r = rand() % (sizeof(animations)/sizeof(animations[0]));
        game->animation = &animations[r];
        audio_play(&game->audio, AUDIO_STREAM__BATTLE);
        // TODO instead of sleeping, need to either:
        // 1) start audio immediately, or
        // 2) run the following code when the playback callback is first triggered
        sleep(1);
        game->saved_frame = game->frame;
        break;
    case GAME_STATE__BATTLE:
        game->battle = battle_new(game->player);
        snprintf(game->debug, sizeof(game->debug), "It's battle time.");
        // TODO audio_play + seek to offset of pre-battle animation audio?
        game->saved_frame = game->frame;
        break;
    }

    game->state = state;
}

void game_render(struct game *game)
{
    WINDOW *main_win = stdscr;
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
                    attron(COLOR_PAIR(COLOR__GRASS));
                    mvaddch(y, x, TILE_GRASS);
                    attroff(COLOR_PAIR(COLOR__GRASS));
                    break;
                }
            }
        }
        mvaddch(game->player->y, game->player->x, '@');
        break;
    case GAME_STATE__PRE_BATTLE:
        box(stdscr, 0, 0);
        int frame = game->frame - game->saved_frame;
        game->animation->render_func(game->animation, frame, h, w);
        break;
    case GAME_STATE__BATTLE:
        // TODO refactor this!!!! (maybe battle_render?)
        main_win = game->windows[WINDOW__BATTLE_PRIMARY];
        int main_h = h * (float)2 / 3;
        // TODO only do resize + mv on screen size change SIGNAL
        wresize(main_win, main_h, w);
        werase(main_win);
        box(main_win, 0, 0);
        WINDOW *sub_win = game->windows[WINDOW__BATTLE_SECONDARY];
        int sub_h = h * (float)1 / 3 + 1;
        wresize(sub_win, sub_h, w);
        werase(sub_win);
        mvwin(sub_win, main_h, 0);
        box(sub_win, 0, 0);
        // draw portraits of battle subjects
        int sprite_w = 20;
        int sprite_h = main_h / 3;
        int padding_w = w / 30;
        int padding_h = 2;
        // TODO saved_frame wouldn't work here if abilities were animated
        float ratio = (float)(game->frame - game->saved_frame) / 15;
        struct monster *monster_top = game->battle->monster_top;
        if (ratio > 1) {
            ratio = 1;
            // animation finished, show names, level, and hp bars
            // top subject
            mvwprintw(main_win, padding_h*2, padding_w*3, "%s (lv %d)",
                    monster_top->species->name, monster_top->level);
            mvwprintw(main_win, padding_h*2 + 1, padding_w*3 + 2, "HP:");
            int hp_w = w / 4;
            float hp_ratio = (float)monster_top->hp / monster_top->hp_total;
            int hp_remaining_w = hp_w * hp_ratio;
            if (hp_ratio > .5f) {
                wattron(main_win, COLOR_PAIR(COLOR__HP_GOOD));
            } else if (hp_ratio > .25f) {
                wattron(main_win, COLOR_PAIR(COLOR__HP_WARNING));
            } else {
                wattron(main_win, COLOR_PAIR(COLOR__HP_DANGER));
            }
            for (int x = 0; x < hp_remaining_w; ++x)
                mvwaddch(main_win, padding_h*2 + 1, padding_w*3 + 2 + sizeof("HP:") + x, ACS_CKBOARD);
            if (hp_ratio > .5f) {
                wattroff(main_win, COLOR_PAIR(COLOR__HP_GOOD));
            } else if (hp_ratio > .25f) {
                wattroff(main_win, COLOR_PAIR(COLOR__HP_WARNING));
            } else {
                wattroff(main_win, COLOR_PAIR(COLOR__HP_DANGER));
            }
            for (int x = hp_remaining_w; x < hp_w; ++x)
                mvwaddch(main_win, padding_h*2 + 1,
                        padding_w*3 + 2 + sizeof("HP:") + x, ACS_CKBOARD);
            mvwprintw(main_win, padding_h*2 + 2,
                    padding_w*3 + 2 + sizeof("HP:"), "%d / %d",
                    monster_top->hp, monster_top->hp_total);

            // bottom subject (TODO monster_battle_render or something)
            struct monster *monster_bot = game->battle->monster_bot;
            mvwprintw(main_win, main_h - padding_h*2 - 2,
                    w - padding_w*3 - hp_w - 2, "%s (lv %d)",
                    monster_bot->species->name, monster_bot->level);
            mvwprintw(main_win, main_h - padding_h*2 - 1,
                    w - padding_w*3 - hp_w, "HP:");
            hp_ratio = (float)monster_bot->hp / monster_bot->hp_total;
            hp_remaining_w = hp_w * hp_ratio;
            if (hp_ratio > .5f) {
                wattron(main_win, COLOR_PAIR(COLOR__HP_GOOD));
            } else if (hp_ratio > .25f) {
                wattron(main_win, COLOR_PAIR(COLOR__HP_WARNING));
            } else {
                wattron(main_win, COLOR_PAIR(COLOR__HP_DANGER));
            }
            for (int x = 0; x < hp_remaining_w; ++x)
                mvwaddch(main_win, main_h - padding_h*2 - 1,
                        w - padding_w*3 - hp_w + sizeof("HP:") + x, ACS_CKBOARD);
            if (hp_ratio > .5f) {
                wattroff(main_win, COLOR_PAIR(COLOR__HP_GOOD));
            } else if (hp_ratio > .25f) {
                wattroff(main_win, COLOR_PAIR(COLOR__HP_WARNING));
            } else {
                wattroff(main_win, COLOR_PAIR(COLOR__HP_DANGER));
            }
            for (int x = hp_remaining_w; x < hp_w; ++x)
                mvwaddch(main_win, main_h - padding_h*2 - 1,
                        w - padding_w*3 - hp_w + sizeof("HP:") + x, ACS_CKBOARD);
            mvwprintw(main_win, main_h - padding_h*2,
                    w - padding_w*3 - hp_w + sizeof("HP:"), "%d / %d",
                    monster_bot->hp, monster_bot->hp_total);

            // TODO make this "responsive" (tongue-in-cheek)
            for (int y = 0; y < sub_h; ++y)
                mvwaddch(sub_win, y, w - 44, '|');

            mvwprintw(sub_win, 3, w - 34, "[F]IGHT");
            mvwprintw(sub_win, 3, w - 14, "P[K]MN");
            mvwprintw(sub_win, sub_h - 4, w - 34, "[P]ACK");
            mvwprintw(sub_win, sub_h - 4, w - 14, "[R]UN");
        } else {
            mvwprintw(sub_win, 2, 6, "WILD %s APPEARED!", monster_top->species->name);
        }
        int sprite_x_offset = (w - sprite_w - padding_w) * ratio;
        for (int y = 0; y < sprite_h; ++y) {
            for (int x = 0; x < sprite_w; ++x) {
                // top sprite
                mvwaddch(main_win, padding_h + y, sprite_x_offset + x, ACS_CKBOARD);
                // bottom sprite
                mvwaddch(main_win, main_h - padding_h - sprite_h + y, w - sprite_x_offset - x, ACS_CKBOARD);
            }
        }
        wrefresh(sub_win);
        break;
    }

    mvwprintw(main_win, 0, 0, game->debug);
    wrefresh(main_win);
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
    // TODO only support these if game state is GAME_STATE__BATTLE
    case 'h':
        game->battle->monster_top->hp -= 3; // TODO
        break;
    case 'f':
        // TODO show fight menu
        break;
    case 'k':
        // TODO pkmn menu
        break;
    case 'p':
        // TODO pack menu
        break;
    case 'r':
        // TODO chance of not running... print message, animate out?
        game_set_state(game, GAME_STATE__EXPLORE);
        break;
    // XXX DEBUG these are simply for debugging purposes (and should be removed)
    case '1':
    case '2':
    case '3':
        game_set_state(game, c - '1');
        break;
    }
}
