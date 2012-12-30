#include <stdlib.h>
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>

#include "seqview.h"
#include "sequencer.h"
#include "config.h"
#include "util.h"

seqview_block_t* block_play_pause;
seqview_block_t* block_stop;
seqview_block_t* block_clock_bpm;
seqview_block_t* block_clock_sync;
seqview_block_t* block_tracks;

seqview_t* seqview_new(SDL_Surface* screen, sequencer_t* sequencer) {
    seqview_t* seqview;

    seqview = (seqview_t*)calloc(1, sizeof(seqview_t));

    seqview->sequencer = sequencer;

    seqview->num_blocks = SEQVIEW_BLOCK_INDEX_TRACKS // Global
        + (sequencer->num_tracks * (SEQVIEW_BLOCK_OFFSET_TRACK_STEPS)) // Track controls
        + (sequencer->num_tracks * sequencer->max_steps_per_track); // Track steps

    seqview->blocks = calloc(seqview->num_blocks, sizeof(seqview_block_t));

    seqview->font = TTF_OpenFont("font.ttf", config_get("seqview.font_size", 12));

    seqview->block_width = config_get("seqview.step_size", 48);
    seqview->block_padding = config_get("seqview.step_padding", 4);
    seqview->block_size = seqview->block_width + seqview->block_padding;

    seqview_init_block_global(seqview);
    seqview_init_block_tracks(seqview);

    // Create surface
    seqview->surface = SDL_CreateRGBSurface(
        SDL_HWSURFACE,
        seqview->block_size * MAX(SEQVIEW_BLOCK_INDEX_TRACKS + 1, sequencer->max_steps_per_track),
        seqview->block_size * (1 + 2 * sequencer->num_tracks),
        screen->format->BitsPerPixel,
        screen->format->Rmask,
        screen->format->Gmask,
        screen->format->Bmask,
        screen->format->Amask
    );

    seqview->color_step_active = SDL_MapRGB(seqview->surface->format, SEQVIEW_COLOR_STEP_ACTIVE);
    seqview->color_step_inactive = SDL_MapRGB(seqview->surface->format, SEQVIEW_COLOR_STEP_INACTIVE);
    seqview->color_background = SDL_MapRGB(seqview->surface->format, SEQVIEW_COLOR_BACKGROUND);
    (seqview->color_font).r = 0xff;
    (seqview->color_font).g = 0xff;
    (seqview->color_font).b = 0xff;

    (seqview->viewport_rect).x = 0;
    (seqview->viewport_rect).y = 0;
    (seqview->viewport_rect).w = screen->w;
    (seqview->viewport_rect).h = screen->h;

    return seqview;
}

int seqview_init_block_global(seqview_t* seqview) {

    block_play_pause = &seqview->blocks[SEQVIEW_BLOCK_INDEX_PLAY_PAUSE];
    sprintf(block_play_pause->label, "Play");
    block_play_pause->left = seqview->block_size * SEQVIEW_BLOCK_INDEX_PLAY_PAUSE;
    block_play_pause->is_button = TRUE;

    block_stop = &seqview->blocks[SEQVIEW_BLOCK_INDEX_STOP];
    sprintf(block_stop->label, "Stop");
    block_stop->left = seqview->block_size * SEQVIEW_BLOCK_INDEX_STOP;
    block_stop->is_button = TRUE;

    block_clock_bpm = &seqview->blocks[SEQVIEW_BLOCK_INDEX_CLOCK_BPM];
    sprintf(block_clock_bpm->label, "BPM");
    block_clock_bpm->left = seqview->block_size * SEQVIEW_BLOCK_INDEX_CLOCK_BPM;

    block_clock_sync = &seqview->blocks[SEQVIEW_BLOCK_INDEX_CLOCK_SYNC];
    sprintf(block_clock_sync->label, "Sync");
    sprintf(block_clock_sync->value, "(none)");
    block_clock_sync->left = seqview->block_size * SEQVIEW_BLOCK_INDEX_CLOCK_SYNC;

    return 0;
}

int seqview_init_block_tracks(seqview_t* seqview) {
    int track_i;
    int block_i;
    int step_i;
    int step_block_i;
    seqview_block_t* block;
    track_t* track;

    block_tracks = &seqview->blocks[SEQVIEW_BLOCK_INDEX_TRACKS];

    for (track_i = 0; track_i < seqview->sequencer->num_tracks; track_i++) {

        block_i = SEQVIEW_BLOCK_INDEX_TRACKS
            + (track_i * (SEQVIEW_BLOCK_OFFSET_TRACK_STEPS + seqview->sequencer->max_steps_per_track));

        block = &seqview->blocks[block_i + SEQVIEW_BLOCK_OFFSET_TRACK_CTL_DEVICE];
        sprintf(block->label, "Device");
        block->top = seqview->block_size * (1 + (track_i * 2));
        block->left = seqview->block_size * SEQVIEW_BLOCK_OFFSET_TRACK_CTL_DEVICE;

        block = &seqview->blocks[block_i + SEQVIEW_BLOCK_OFFSET_TRACK_CTL_CHANNEL];
        sprintf(block->label, "Channel");
        block->top = seqview->block_size * (1 + (track_i * 2));
        block->left = seqview->block_size * SEQVIEW_BLOCK_OFFSET_TRACK_CTL_CHANNEL;

        block = &seqview->blocks[block_i + SEQVIEW_BLOCK_OFFSET_TRACK_CTL_LENGTH];
        sprintf(block->label, "Length");
        block->top = seqview->block_size * (1 + (track_i * 2));
        block->left = seqview->block_size * SEQVIEW_BLOCK_OFFSET_TRACK_CTL_LENGTH;

        block = &seqview->blocks[block_i + SEQVIEW_BLOCK_OFFSET_TRACK_CTL_STRIDE];
        sprintf(block->label, "Stride");
        block->top = seqview->block_size * (1 + (track_i * 2));
        block->left = seqview->block_size * SEQVIEW_BLOCK_OFFSET_TRACK_CTL_STRIDE;

        block = &seqview->blocks[block_i + SEQVIEW_BLOCK_OFFSET_TRACK_CTL_OFFSET];
        sprintf(block->label, "Offset");
        block->top = seqview->block_size * (1 + (track_i * 2));
        block->left = seqview->block_size * SEQVIEW_BLOCK_OFFSET_TRACK_CTL_OFFSET;

        block = &seqview->blocks[block_i + SEQVIEW_BLOCK_OFFSET_TRACK_CTL_CLOCKDIV];
        sprintf(block->label, "Clockdiv");
        block->top = seqview->block_size * (1 + (track_i * 2));
        block->left = seqview->block_size * SEQVIEW_BLOCK_OFFSET_TRACK_CTL_CLOCKDIV;

        block = &seqview->blocks[block_i + SEQVIEW_BLOCK_OFFSET_TRACK_CTL_TRANSPOSE];
        sprintf(block->label, "Transpose");
        block->top = seqview->block_size * (1 + (track_i * 2));
        block->left = seqview->block_size * SEQVIEW_BLOCK_OFFSET_TRACK_CTL_TRANSPOSE;

        block = &seqview->blocks[block_i + SEQVIEW_BLOCK_OFFSET_TRACK_CTL_VELOCITY];
        sprintf(block->label, "Velocity");
        block->top = seqview->block_size * (1 + (track_i * 2));
        block->left = seqview->block_size * SEQVIEW_BLOCK_OFFSET_TRACK_CTL_VELOCITY;

        block = &seqview->blocks[block_i + SEQVIEW_BLOCK_OFFSET_TRACK_CTL_GATE];
        sprintf(block->label, "Gate");
        block->top = seqview->block_size * (1 + (track_i * 2));
        block->left = seqview->block_size * SEQVIEW_BLOCK_OFFSET_TRACK_CTL_GATE;

        track = &seqview->sequencer->tracks[track_i];
        for (step_i = 0; step_i < seqview->sequencer->max_steps_per_track; step_i++) {
            block = &seqview->blocks[block_i + SEQVIEW_BLOCK_OFFSET_TRACK_STEPS + step_i];
            block->top = seqview->block_size * (1 + (track_i * 2 + 1));
            block->left = seqview->block_size * step_i;
            block->is_hidden = (step_i >= track->num_steps);
            block->step = &track->steps[step_i];
        }
    }

    return 0;
}

int seqview_render(seqview_t* seqview, SDL_Surface* screen) {
    seqview_block_t* block;
    int block_i;

    // Only worry about visible portion of surface
    SDL_SetClipRect(seqview->surface, &seqview->viewport_rect);

    // Draw background
    seqview_fill_rect(
        seqview->surface,
        0,
        0,
        seqview->surface->w,
        seqview->surface->h,
        seqview->color_background
    );

    // Draw blocks
    for (block_i = 0; block_i < seqview->num_blocks; block_i++) {
        block = &seqview->blocks[block_i];
        if (!block->is_hidden) {
            seqview_block_render(block, seqview);
        }
    }

    // Draw to screen
    SDL_BlitSurface(
        seqview->surface,
        &seqview->viewport_rect,
        screen,
        NULL
    );

    // Flip buffer
    SDL_Flip(screen);

    return 0;
}

int seqview_fill_rect(SDL_Surface* dest, int x, int y, int w, int h, int color) {
    SDL_Rect rect = {x, y, w, h};
    return SDL_FillRect(dest, &rect, color);
}

int seqview_block_render(seqview_block_t* self, seqview_t* seqview) {
    SDL_Rect rect_src;
    SDL_Rect rect_dest;
    Uint32 color;

    if (self->step && self->step->track->active_step == self->step) {
        color = seqview->color_step_active;
    } else {
        color = seqview->color_step_inactive;
    }

    seqview_fill_rect(
        seqview->surface,
        seqview->block_padding + self->left,
        seqview->block_padding + self->top,
        seqview->block_width,
        seqview->block_width,
        color
    );

    if (!self->label_surface && strlen(self->label) > 0) {
        self->label_surface = TTF_RenderText_Solid(
            seqview->font,
            &self->label,
            seqview->color_font
        );
    }

    if (self->label_surface) {
        rect_src.x = 0;
        rect_src.y = 0;
        rect_src.w = MIN(self->label_surface->w, seqview->block_width - 2);
        rect_src.h = self->label_surface->h;
        rect_dest.x = seqview->block_padding + self->left + 1;
        rect_dest.y = seqview->block_padding + self->top;
        rect_dest.w = seqview->block_width;
        rect_dest.h = seqview->block_width;
        SDL_BlitSurface(
            self->label_surface,
            &rect_src,
            seqview->surface,
            &rect_dest
        );
    }

    return 0;
}
