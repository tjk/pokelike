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
    for (int i = 0; i < NUM__WINDOW; ++i)
        game->windows[i] = newwin(0, 0, 0, 0);
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
        game->cursor = CURSOR__FIGHT;
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

    if (h < 25 || w < 80) {
        mvwprintw(main_win, 0, 0, "Terminal must be at least 80x25 (%dx%d)", w, h);
        wrefresh(main_win);
        return;
    }

    h = 25;
    w = 80;
    wresize(main_win, h, w);

    switch (game->state) {
    case GAME_STATE__EXPLORE:
        box(stdscr, 0, 0);
        for (int y = 0; y < MAP_CHUNK_HEIGHT; ++y) {
            for (int x = 0; x < MAP_CHUNK_WIDTH; ++x) {
                switch (game->chunk->tiles[y][x]) {
                case TILE_GRASS:
                    wattron(main_win, COLOR_PAIR(COLOR__GRASS));
                    mvaddch(y, x, TILE_GRASS);
                    wattroff(main_win, COLOR_PAIR(COLOR__GRASS));
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
        int sprite_w = MONSTER_SPECIES_BITMAP_W;
        int sprite_h = MONSTER_SPECIES_BITMAP_H;
        int padding_w = 5;
        int padding_h = 2;
        // TODO saved_frame wouldn't work here if abilities were animated
        float ratio = (float)(game->frame - game->saved_frame) / 15;
        struct monster *monster_top = game->battle->monster_top;
        struct monster *monster_bot = game->battle->monster_bot;
        // TODO state based on frame ratio... not good!
        if (ratio >= 1) {
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

#define MAIN_MENU_ITEM(n, on, y, x) do { \
    if ((on) || (game->cursor == CURSOR__##n && game->frame % 6 < 4)) \
    wattron(sub_win, A_REVERSE); \
    mvwprintw(sub_win, (y), (x), #n); \
    if ((on) || (game->cursor == CURSOR__##n && game->frame % 6 < 4)) \
        wattroff(sub_win, A_REVERSE); \
} while (0)
            MAIN_MENU_ITEM(FIGHT, game->cursor == CURSOR__FIGHT, 2,  5);
            MAIN_MENU_ITEM(TEAM,  false, 3,  5);
            MAIN_MENU_ITEM(PACK,  false, 4,  5);
            MAIN_MENU_ITEM(RUN,   false, 6,  5);
            // TODO make this "responsive" (tongue-in-cheek)
            for (int y = 0; y < sub_h; ++y)
                mvwaddch(sub_win, y, 15, '|');

            switch (game->battle->state) {
            case BATTLE_STATE__FIGHT:
#define FIGHT_MOVE(i, name, description, y, x) do { \
    if (game->cursor == CURSOR__FIGHT_MOVE + i && game->frame % 6 < 4) \
    wattron(sub_win, A_REVERSE); \
    mvwprintw(sub_win, (y), (x), (name)); \
    if (game->cursor == CURSOR__FIGHT_MOVE + i && game->frame % 6 < 4) \
        wattroff(sub_win, A_REVERSE); \
    if (game->cursor == CURSOR__FIGHT_MOVE + i) \
        mvwprintw(sub_win, (y) + 3, 20, (description)); \
} while (0)
                    for (int i = 0; i < ARRAY_SIZE(monster_bot->moves); ++i) {
                        if (monster_bot->moves[i]) {
                            FIGHT_MOVE(i, monster_bot->moves[i]->name,
                                    monster_bot->moves[i]->description, 2, 20+i*5);
                        }
                    }
                break;
            }
        } else {
            mvwprintw(sub_win, 2, 6, "WILD %s APPEARED!", monster_top->species->name);
        }
        int sprite_x_offset = (w - sprite_w - padding_w) * ratio;
        // top sprite
        wattron(main_win, COLOR_PAIR(monster_top->species->color));
        for (int y = 0; y < sprite_h; ++y) {
            for (int x = 0; x < sprite_w; ++x) {
                if (monster_top->species->bitmap[y][x])
                    mvwaddch(main_win, padding_h + y, sprite_x_offset + x, ACS_CKBOARD);
            }
        }
        wattroff(main_win, COLOR_PAIR(monster_top->species->color));
        // bottom sprite
        wattron(main_win, COLOR_PAIR(monster_bot->species->color));
        for (int y = 0; y < sprite_h; ++y) {
            for (int x = 0; x < sprite_w; ++x) {
                if (monster_bot->species->bitmap[y][x])
                    mvwaddch(main_win, main_h - padding_h - sprite_h + y, w - sprite_x_offset - x, ACS_CKBOARD);
            }
        }
        wattroff(main_win, COLOR_PAIR(monster_bot->species->color));
        wrefresh(sub_win);
        break;
    }

    mvwprintw(main_win, 0, 0, game->debug);
    wrefresh(main_win);
}

// TODO for menu, a "UI pattern" may be better (widgets are linked, different
// inputs traverse links to other widgets)
void game_handle_input(struct game *game, int c)
{
    switch (game->state) {
    case GAME_STATE__EXPLORE:
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
        break;
    case GAME_STATE__BATTLE:
        switch (game->cursor) {
        case CURSOR__FIGHT:
            switch (c) {
            case 13: // ENTER
            case KEY_RIGHT:
                game->battle->state = BATTLE_STATE__FIGHT;
                game->cursor = CURSOR__FIGHT_MOVE_1;
                break;
            case KEY_UP:
                game->cursor = CURSOR__RUN;
                break;
            case KEY_DOWN:
                game->cursor = CURSOR__TEAM;
                break;
            }
            break;
        case CURSOR__TEAM:
            switch (c) {
            // TODO case ENTER
            case KEY_RIGHT:
                break;
            case KEY_UP:
                game->cursor = CURSOR__FIGHT;
                break;
            case KEY_DOWN:
                game->cursor = CURSOR__PACK;
                break;
            }
            break;
        case CURSOR__PACK:
            switch (c) {
            // TODO case ENTER
            case KEY_RIGHT:
                break;
            case KEY_UP:
                game->cursor = CURSOR__TEAM;
                break;
            case KEY_DOWN:
                game->cursor = CURSOR__RUN;
                break;
            }
            break;
        case CURSOR__RUN:
            switch (c) {
            case 13: // ENTER
                game_set_state(game, GAME_STATE__EXPLORE);
                break;
            case KEY_UP:
                game->cursor = CURSOR__PACK;
                break;
            case KEY_DOWN:
                game->cursor = CURSOR__FIGHT;
                break;
            }
            break;
        case CURSOR__FIGHT_MOVE_1:
        case CURSOR__FIGHT_MOVE_2:
        case CURSOR__FIGHT_MOVE_3:
        case CURSOR__FIGHT_MOVE_4:;
            int cursor = game->cursor - CURSOR__FIGHT_MOVE;
            switch (c) {
            case 13: // ENTER
                game->battle->move_bot = game->battle->monster_bot->moves[cursor];
                int rmove = rand() % game->battle->monster_top->num_moves;
                game->battle->move_top = game->battle->monster_top->moves[rmove];
                // TODO game->battle->state = BATTLE_STATE__FIGHTING;
                // NOTE for now, just apply the moves (REFACTOR!!!)
                game->battle->monster_bot->hp -= game->battle->move_top->base_dmg;
                game->battle->monster_top->hp -= game->battle->move_bot->base_dmg;
                break;
            case KEY_LEFT:
                cursor -= 1;
                game->cursor = cursor < CURSOR__FIGHT_MOVE_1 ?
                    CURSOR__FIGHT : cursor;
                game->battle->state = BATTLE_STATE__MAIN_MENU;
                break;
            case KEY_RIGHT:
                cursor += 1;
                if (cursor < game->battle->monster_bot->num_moves - 1)
                    game->cursor = cursor;
                break;
            }
            break;
        }
        break;
    default:
        break;
    }

    // XXX DEBUG these are simply for debugging purposes (and should be removed)
    switch (c) {
    case '1':
    case '2':
    case '3':
        game_set_state(game, c - '1');
        break;
    }
}
