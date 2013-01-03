#include <stdlib.h>
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include "utlist.h"

#include "seqview.h"
#include "sequencer.h"
#include "config.h"
#include "util.h"

/**
 * Make a new seqview
 *
 * @param SDL_Surface* screen
 * @param sequencer_t* sequencer
 * @return seqview_t*
 */
seqview_t* seqview_new(SDL_Surface* screen, sequencer_t* sequencer) {
    seqview_t* seqview;

    seqview = (seqview_t*)calloc(1, sizeof(seqview_t));

    seqview->sequencer = sequencer;

    seqview->num_blocks = SEQVIEW_BLOCK_INDEX_TRACKS // Global
        + (sequencer->num_tracks * (SEQVIEW_BLOCK_OFFSET_TRACK_STEPS)) // Track controls
        + (sequencer->num_tracks * sequencer->max_steps_per_track); // Track steps

    seqview->blocks = (seqview_block_t*)calloc(seqview->num_blocks, sizeof(seqview_block_t));

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

    SEQVIEW_COLOR_INIT(seqview, step_active, 0xc0, 0xc0, 0xc0);
    SEQVIEW_COLOR_INIT(seqview, step_inactive, 0x80, 0x80, 0x80);
    SEQVIEW_COLOR_INIT(seqview, step_focus, 0xe0, 0x60, 0x60);
    SEQVIEW_COLOR_INIT(seqview, step_input, 0x60, 0xe0, 0x60);
    SEQVIEW_COLOR_INIT(seqview, background, 0x30, 0x30, 0x30);
    SEQVIEW_COLOR_INIT(seqview, font, 0xff, 0xff, 0xff);

    (seqview->viewport_rect).x = 0;
    (seqview->viewport_rect).y = 0;
    (seqview->viewport_rect).w = screen->w;
    (seqview->viewport_rect).h = screen->h;

    return seqview;
}

/**
 * Init global control
 *
 * @param seqview_t* seqview
 */
int seqview_init_block_global(seqview_t* seqview) {

    seqview->block_play_pause = &seqview->blocks[SEQVIEW_BLOCK_INDEX_PLAY_PAUSE];
    sprintf(seqview->block_play_pause->label, "Play");
    seqview->block_play_pause->left = seqview->block_size * SEQVIEW_BLOCK_INDEX_PLAY_PAUSE;
    seqview->block_play_pause->is_button = TRUE;

    seqview->block_stop = &seqview->blocks[SEQVIEW_BLOCK_INDEX_STOP];
    sprintf(seqview->block_stop->label, "Stop");
    seqview->block_stop->left = seqview->block_size * SEQVIEW_BLOCK_INDEX_STOP;
    seqview->block_stop->is_button = TRUE;

    seqview->block_clock_bpm = &seqview->blocks[SEQVIEW_BLOCK_INDEX_CLOCK_BPM];
    sprintf(seqview->block_clock_bpm->label, "BPM");
    seqview->block_clock_bpm->left = seqview->block_size * SEQVIEW_BLOCK_INDEX_CLOCK_BPM;

    seqview->block_clock_sync = &seqview->blocks[SEQVIEW_BLOCK_INDEX_CLOCK_SYNC];
    sprintf(seqview->block_clock_sync->label, "Sync");
    sprintf(seqview->block_clock_sync->value, "(none)");
    seqview->block_clock_sync->left = seqview->block_size * SEQVIEW_BLOCK_INDEX_CLOCK_SYNC;

    return 0;
}

/**
 * Init track controls and track steps
 *
 * @param seqview_t* seqview
 */
int seqview_init_block_tracks(seqview_t* seqview) {
    int track_i;
    int block_i;
    int step_i;
    seqview_block_t* block;
    track_t* track;

    seqview->block_tracks = &seqview->blocks[SEQVIEW_BLOCK_INDEX_TRACKS];

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

        block = &seqview->blocks[block_i + SEQVIEW_BLOCK_OFFSET_TRACK_CTL_TYPE];
        sprintf(block->label, "Type");
        block->top = seqview->block_size * (1 + (track_i * 2));
        block->left = seqview->block_size * SEQVIEW_BLOCK_OFFSET_TRACK_CTL_TYPE;

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
            block->step_index = step_i;
            block->step = &track->steps[step_i];
        }
    }

    return 0;
}

/**
 * Render seqview
 *
 * @param seqview_t* seqview
 * @param SDL_Surface* screen
 */
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
        seqview->color_background_i
    );

    // Draw blocks
    for (block_i = 0; block_i < seqview->num_blocks; block_i++) {
        block = &seqview->blocks[block_i];
        if (!block->step || block->step_index < block->step->track->num_steps) {
            seqview_render_block(block, seqview);
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

/**
 * Render a single block
 *
 * @param seqview_block_t* self
 * @param seqview_t* seqview
 */
int seqview_render_block(seqview_block_t* self, seqview_t* seqview) {
    SDL_Rect rect_src;
    SDL_Rect rect_dest;
    int color;

    if (self->step && self->step->track->active_step == self->step) {
        color = seqview->color_step_active_i;
    } else {
        color = seqview->color_step_inactive_i;
    }

    seqview_fill_rect(
        seqview->surface,
        seqview->block_padding + self->left,
        seqview->block_padding + self->top,
        seqview->block_width,
        seqview->block_width,
        color
    );

    if (seqview->block_focus == self) {
        seqview_fill_rect(
            seqview->surface,
            seqview->block_padding + self->left,
            seqview->block_padding + self->top + 3 * (seqview->block_width / 4),
            seqview->block_width,
            (seqview->block_width / 4),
            seqview->color_step_focus_i
        );
    }

    if (self->input_buffer_len > 0) {
        seqview_fill_rect(
            seqview->surface,
            seqview->block_padding + self->left,
            seqview->block_padding + self->top + 2 * (seqview->block_width / 4),
            seqview->block_width,
            (seqview->block_width / 4),
            seqview->color_step_input_i
        );
    }

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

/**
 * Handle keyboard input
 *
 * @param seqview_t* seqview
 * @param SDL_KeyboardEvent key
 */
int seqview_handle_input_key(seqview_t* seqview, SDL_KeyboardEvent key) {

    if (seqview->block_focus
        && seqview->block_focus->input_buffer_len < sizeof(seqview->block_focus->input_buffer)
    ) {
        if (key.keysym.sym == SDLK_BACKSPACE
            || key.keysym.sym == SDLK_DELETE
        ) {
            if (seqview->block_focus->input_buffer_len > 0) {
                seqview->block_focus->input_buffer[
                    seqview->block_focus->input_buffer_len - 1
                ] = '\0';
                seqview->block_focus->input_buffer_len -= 1;
            }
        /*} else if (key.keysym.sym == SDLK_LEFT) {
            seqview_move_focus
        } else if (key.keysym.sym == SDLK_RIGHT) {
        } else if (key.keysym.sym == SDLK_UP) {
        } else if (key.keysym.sym == SDLK_DOWN) {
        } else if (key.keysym.sym == SDLK_RETURN) {
            seqview_block_execute_input(seqview, seqview->block_focus);
            */
        } else if ((key.keysym.unicode & 0xff80) == 0) { // ASCII
            seqview->block_focus->input_buffer[
                seqview->block_focus->input_buffer_len
            ] = (char)(key.keysym.unicode & 0x7f);
            seqview->block_focus->input_buffer_len += 1;
        }
        fprintf(stderr, "input_buffer=%s\n", seqview->block_focus->input_buffer);
    }
}

/**
 * Handle mouse input
 *
 * @param seqview_t* seqview
 * @param SDL_MouseButtonEvent button
 */
int seqview_handle_input_mouse(seqview_t* seqview, SDL_MouseButtonEvent button) {
    seqview_block_t* block;
    seqview_block_stack_t* block_stack_item;

    block = seqview_get_block_by_coordinate(seqview, button.x, button.y);

    /*if (button.state == SDL_PRESSED && block->step) {
        track_set_length(
            block->step->track,
            block->step->track->num_steps + (button.button == 1 ? 1 : -1)
        );
    }*/

    if (block && button.state == SDL_PRESSED) {
        block_stack_item = (seqview_block_stack_t*)calloc(1, sizeof(seqview_block_stack_t));
        block_stack_item->block = block;
        block->is_button_down = TRUE;
        seqview->block_focus = block;
        DL_APPEND(seqview->block_down_stack, block_stack_item);
    } else if (
        button.state == SDL_RELEASED
        && seqview->block_down_stack
        && seqview->block_down_stack->prev
    ) {
        seqview->block_down_stack->prev->block->is_button_down = FALSE;
        block_stack_item = seqview->block_down_stack->prev;
        DL_DELETE(seqview->block_down_stack, seqview->block_down_stack->prev);
        free(block_stack_item);
    }
    return 0;
}

/**
 * Execute command in block's input buffer
 *
 * @param seqview_t* seqview
 * @param seqview_block_t* block
 */
int seqview_block_execute_input(seqview_t* seqview, seqview_block_t* block) {

}

/**
 * Given an x,y coordinate, get the corresponding block if any
 *
 * @param seqview_t* seqview
 * @param int x
 * @param int y
 * @return seqview_block_t*
 */
seqview_block_t* seqview_get_block_by_coordinate(seqview_t* seqview, int x, int y) {
    int x_block;
    int y_block;
    int track_i;
    bool is_track_ctl_row;
    bool is_track_step_row;

    x_block = x / seqview->block_size;
    y_block = y / seqview->block_size;
    is_track_ctl_row = (y_block > 0 && y_block % 2 == 1);
    is_track_step_row = (y_block > 1 && y_block % 2 == 0);
    track_i = (y_block - 1) / 2;

    if (y_block == 0
        && x_block >= 0
        && x_block < SEQVIEW_BLOCK_INDEX_TRACKS
    ) {
        // Global control
        return &seqview->blocks[x_block];
    } else if (
        is_track_ctl_row
        && track_i < seqview->sequencer->num_tracks
        && x_block >= 0
        && x_block < SEQVIEW_BLOCK_OFFSET_TRACK_STEPS
    ) {
        // Track control
        return &seqview->blocks[
            SEQVIEW_BLOCK_INDEX_TRACKS
            + (track_i * (SEQVIEW_BLOCK_OFFSET_TRACK_STEPS + seqview->sequencer->max_steps_per_track))
            + x_block
        ];
    } else if (
        is_track_step_row
        && track_i < seqview->sequencer->num_tracks
        && x_block >= 0
        && x_block < seqview->sequencer->max_steps_per_track
    ) {
        // Track step
        return &seqview->blocks[
            SEQVIEW_BLOCK_INDEX_TRACKS
            + (track_i * (SEQVIEW_BLOCK_OFFSET_TRACK_STEPS + seqview->sequencer->max_steps_per_track))
            + SEQVIEW_BLOCK_OFFSET_TRACK_STEPS
            + x_block
        ];
    }
    return NULL;
}

/**
 * Draws a rectangle on dest
 *
 * @param SDL_Surface* dest
 * @param int x
 * @param int y
 * @param int w
 * @param int h
 * @param int color
 */
int seqview_fill_rect(SDL_Surface* dest, int x, int y, int w, int h, int color) {
    SDL_Rect rect = {x, y, w, h};
    return SDL_FillRect(dest, &rect, color);
}
