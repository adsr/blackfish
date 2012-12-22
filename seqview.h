#ifndef _SEQVIEW_H
#define _SEQVIEW_H

#include <SDL/SDL.h>
#include "sequencer.h"

#define SEQVIEW_COLOR_STEP_ACTIVE 0xc0,0xc0,0xc0
#define SEQVIEW_COLOR_STEP_INACTIVE 0x80,0x80,0x80
#define SEQVIEW_COLOR_BACKGROUND 0x30,0x30,0x30

typedef struct seqview_s {
    SDL_Surface* surface;
    sequencer_t* sequencer;
    SDL_Rect viewport_rect;
    Uint32 color_background;
    Uint32 color_step_active;
    Uint32 color_step_inactive;
    Uint32 color_step_focus;
    int step_size;
    int step_padding;
} seqview_t;

seqview_t* seqview_new(SDL_Surface* screen, sequencer_t* sequencer);
int seqview_render(seqview_t* seqview, SDL_Surface* screen);
int seqview_fill_rect(SDL_Surface* dest, int x, int y, int w, int h, int color);

#endif
