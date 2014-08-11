#pragma once

#include "common.h"

#define NUM_FLICKER_FRAMES 12

struct animation;

typedef void (*animation_render_func)(struct animation *animation, int frame, int h, int w);

struct animation {
    int num_frames;
    animation_render_func render_func;
};

void animation_curtain_render(struct animation *animation, int frame, int h, int w);
void animation_inward_render(struct animation *animation, int frame, int h, int w);
void animation_snake_render(struct animation *animation, int frame, int h, int w);
