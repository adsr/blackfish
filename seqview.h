#ifndef _SEQVIEW_H
#define _SEQVIEW_H

#include <stdbool.h>
#include <SDL/SDL.h>

#include "sequencer.h"
#include "step.h"

#define SEQVIEW_COLOR_DECLARE(COLOR_NAME) \
    SDL_Color color_ ## COLOR_NAME; \
    int color_ ## COLOR_NAME ## _i;

#define SEQVIEW_COLOR_INIT(SEQVIEW, COLOR_NAME, R, G, B) do { \
    (SEQVIEW) ## -> ## color_ ## COLOR_NAME ## .r = config_get("seqview.color_" #COLOR_NAME ".r", (R)); \
    (SEQVIEW) ## -> ## color_ ## COLOR_NAME ## .g = config_get("seqview.color_" #COLOR_NAME ".g", (G)); \
    (SEQVIEW) ## -> ## color_ ## COLOR_NAME ## .b = config_get("seqview.color_" #COLOR_NAME ".b", (B)); \
    (SEQVIEW) ## -> ## color_ ## COLOR_NAME ## _i = SDL_MapRGB( \
        (SEQVIEW)->surface->format, \
        (SEQVIEW) ## -> ## color_ ## COLOR_NAME ## .r, \
        (SEQVIEW) ## -> ## color_ ## COLOR_NAME ## .g, \
        (SEQVIEW) ## -> ## color_ ## COLOR_NAME ## .b \
    ); \
} while(0)

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
    SEQVIEW_BLOCK_OFFSET_TRACK_CTL_TYPE,
    SEQVIEW_BLOCK_OFFSET_TRACK_CTL_LENGTH,
    SEQVIEW_BLOCK_OFFSET_TRACK_CTL_STRIDE,
    SEQVIEW_BLOCK_OFFSET_TRACK_CTL_OFFSET,
    SEQVIEW_BLOCK_OFFSET_TRACK_CTL_CLOCKDIV,
    SEQVIEW_BLOCK_OFFSET_TRACK_CTL_TRANSPOSE,
    SEQVIEW_BLOCK_OFFSET_TRACK_CTL_VELOCITY,
    SEQVIEW_BLOCK_OFFSET_TRACK_CTL_GATE,
    SEQVIEW_BLOCK_OFFSET_TRACK_STEPS
};

typedef enum {
    TYPE_GLOBAL,
    TYPE_TRACK_CTL,
    TYPE_TRACK_STEP
} seqview_block_type_e;

typedef struct seqview_s {
    SDL_Surface* surface;
    sequencer_t* sequencer;
    SDL_Rect viewport_rect;
    struct seqview_block_s* blocks;
    struct seqview_block_s* block_play_pause;
    struct seqview_block_s* block_stop;
    struct seqview_block_s* block_clock_bpm;
    struct seqview_block_s* block_clock_sync;
    struct seqview_block_s* block_tracks;
    struct seqview_block_stack_s* block_down_stack;
    struct seqview_block_s* block_focus;
    int num_blocks;
    SEQVIEW_COLOR_DECLARE(step_active)
    SEQVIEW_COLOR_DECLARE(step_inactive)
    SEQVIEW_COLOR_DECLARE(step_focus)
    SEQVIEW_COLOR_DECLARE(step_input)
    SEQVIEW_COLOR_DECLARE(background)
    SEQVIEW_COLOR_DECLARE(font)
    TTF_Font* font;
    int block_width;
    int block_padding;
    int block_size;
} seqview_t;

typedef struct seqview_block_s {
    SDL_Surface* label_surface;
    char label[16];
    char value[16];
    seqview_block_type_e type;
    int data;
    char input_buffer[256];
    int input_buffer_len;
    bool is_active;
    bool is_awaiting_input;
    bool is_button;
    bool is_button_down;
    int left;
    int top;
    step_t* step;
    int step_index;
} seqview_block_t;

typedef struct seqview_block_stack_s {
    seqview_block_t* block;
    struct seqview_block_stack_s* next;
    struct seqview_block_stack_s* prev;
} seqview_block_stack_t;

seqview_t* seqview_new(SDL_Surface* screen, sequencer_t* sequencer);
int seqview_init_block_global(seqview_t* seqview);
int seqview_init_block_tracks(seqview_t* seqview);
int seqview_render(seqview_t* seqview, SDL_Surface* screen);
int seqview_render_block_global(seqview_block_t* self, seqview_t* seqview);
int seqview_render_block_track_control(seqview_block_t* self, seqview_t* seqview);
int seqview_render_block_track_step(seqview_block_t* self, seqview_t* seqview);
int seqview_fill_rect(SDL_Surface* dest, int x, int y, int w, int h, int color);
int seqview_handle_input_key(seqview_t* seqview, SDL_KeyboardEvent key);
int seqview_handle_input_mouse(seqview_t* seqview, SDL_MouseButtonEvent button);
seqview_block_t* seqview_get_block_by_coordinate(seqview_t* seqview, int x, int y);

#endif
