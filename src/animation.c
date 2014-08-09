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
