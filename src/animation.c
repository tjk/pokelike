#include "animation.h"

void animation_curtain_render(struct animation *animation, int h, int w)
{
    int frame = animation->frame;
    int num_frames = animation->num_frames;

    if (frame <= num_frames || frame % 3 == 0) {
        for (int y = 0; y < (int)(h * (float)frame / num_frames); ++y)
            for (int x = 0; x < w; ++x)
                mvaddch(y, x, ACS_CKBOARD);
    }
}

void animation_inward_render(struct animation *animation, int h, int w)
{
    int frame = animation->frame;
    int num_frames = animation->num_frames;

    if (frame <= num_frames || frame % 3 == 0) {
        for (int y = 0; y < (int)(h * (float)frame / num_frames); ++y) {
            for (int x = 0; x < (int)(w * (float)frame / num_frames); ++x)
                mvaddch(y, (y % 4 == 0 || y % 4 == 1) ? x : w - x, ACS_CKBOARD);
        }
    }
}

void animation_snake_render(struct animation *animation, int h, int w)
{
    int frame = animation->frame;
    int num_frames = animation->num_frames;

    if (frame <= num_frames || frame % 3 == 0) {
        int current_line = h * (float)frame / num_frames;
        for (int y = 0; y < current_line; ++y) {
            if (y == current_line - 1) {
                for (int x = 0; x < (int)(w * (float)frame / num_frames); ++x)
                    mvaddch(y, (y % 4 == 0 || y % 4 == 1) ? x : w - x, ACS_CKBOARD);
            } else {
                for (int x = 0; x < w; ++x)
                    mvaddch(y, x, ACS_CKBOARD);
            }
        }
    }
}
