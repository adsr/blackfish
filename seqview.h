#ifndef _SEQVIEW_H
#define _SEQVIEW_H

#include <SDL/SDL.h>
#include "sequencer.h"

#define SEQVIEW_STEP_SIZE 32
#define SEQVIEW_STEP_PADDING 4
#define SEQVIEW_STEP_COLOR_ACTIVE_RGB 0xc0,0xc0,0xc0
#define SEQVIEW_STEP_COLOR_INACTIVE_RGB 0x80,0x80,0x80
#define SEQVIEW_BACKGROUND_COLOR_RGB 0x30,0x30,0x30

typedef struct seqview_s {
    SDL_Surface* surface;
    sequencer_t* sequencer;
    SDL_Rect viewport_rect;
} seqview_t;

seqview_t* seqview_new(SDL_Surface* screen, sequencer_t* sequencer);
int seqview_render(seqview_t* seqview, SDL_Surface* screen);
int seqview_fill_rect(SDL_Surface* dest, int x, int y, int w, int h, int color);

#endif
