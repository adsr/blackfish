#ifndef _SEQVIEW_H
#define _SEQVIEW_H

#include <stdbool.h>
#include <SDL/SDL.h>

#include "sequencer.h"
#include "step.h"

enum SEQVIEW_BLOCK_INDEX {
    SEQVIEW_BLOCK_INDEX_PLAY_PAUSE,
    SEQVIEW_BLOCK_INDEX_STOP,
    SEQVIEW_BLOCK_INDEX_CLOCK_BPM,
    SEQVIEW_BLOCK_INDEX_CLOCK_SYNC,
    SEQVIEW_BLOCK_INDEX_TRACKS
};

enum SEQVIEW_BLOCK_OFFSET_TRACK_CTL {
    SEQVIEW_BLOCK_OFFSET_TRACK_CTL_DEVICE,
    SEQVIEW_BLOCK_OFFSET_TRACK_CTL_CHANNEL,
    SEQVIEW_BLOCK_OFFSET_TRACK_CTL_LENGTH,
    SEQVIEW_BLOCK_OFFSET_TRACK_CTL_STRIDE,
    SEQVIEW_BLOCK_OFFSET_TRACK_CTL_OFFSET,
    SEQVIEW_BLOCK_OFFSET_TRACK_CTL_CLOCKDIV,
    SEQVIEW_BLOCK_OFFSET_TRACK_CTL_TRANSPOSE,
    SEQVIEW_BLOCK_OFFSET_TRACK_CTL_VELOCITY,
    SEQVIEW_BLOCK_OFFSET_TRACK_CTL_GATE,
    SEQVIEW_BLOCK_OFFSET_TRACK_STEPS
};

#define SEQVIEW_COLOR_STEP_ACTIVE 0xc0,0xc0,0xc0
#define SEQVIEW_COLOR_STEP_INACTIVE 0x80,0x80,0x80
#define SEQVIEW_COLOR_BACKGROUND 0x30,0x30,0x30

typedef struct seqview_s {
    SDL_Surface* surface;
    sequencer_t* sequencer;
    SDL_Rect viewport_rect;
    struct seqview_block_s* blocks;
    int num_blocks;
    Uint32 color_background;
    Uint32 color_step_active;
    Uint32 color_step_inactive;
    Uint32 color_step_focus;
    SDL_Color color_font;
    TTF_Font* font;
    int block_width;
    int block_padding;
    int block_size;
} seqview_t;

typedef struct seqview_block_s {
    SDL_Surface* label_surface;
    char label[16];
    char value[16];
    int data;
    char input_buffer[256];
    int input_buffer_len;
    bool is_active;
    bool is_focus;
    bool is_awaiting_input;
    bool is_button;
    bool is_hidden;
    int left;
    int top;
    step_t* step;
    int (*render)(struct seqview_s* seqview, SDL_Surface* screen);
} seqview_block_t;

seqview_t* seqview_new(SDL_Surface* screen, sequencer_t* sequencer);
int seqview_init_block_global(seqview_t* seqview);
int seqview_init_block_tracks(seqview_t* seqview);
int seqview_render(seqview_t* seqview, SDL_Surface* screen);
int seqview_fill_rect(SDL_Surface* dest, int x, int y, int w, int h, int color);

#endif
