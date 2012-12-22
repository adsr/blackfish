#include <stdlib.h>
#include <SDL/SDL.h>

#include "seqview.h"
#include "sequencer.h"

seqview_t* seqview_new(SDL_Surface* screen, sequencer_t* sequencer) {

    // TODO make configurable
    int surface_width = (SEQVIEW_STEP_PADDING * (DEFAULT_MAX_STEPS_PER_TRACK + 1))
        + (SEQVIEW_STEP_SIZE * DEFAULT_MAX_STEPS_PER_TRACK);
    int surface_height = (SEQVIEW_STEP_PADDING * (DEFAULT_NUM_TRACKS + 1))
        + (SEQVIEW_STEP_SIZE * DEFAULT_NUM_TRACKS);

    seqview_t* seqview = (seqview_t*)calloc(1, sizeof(seqview_t));
    seqview->sequencer = sequencer;
    seqview->surface = SDL_CreateRGBSurface(
        SDL_HWSURFACE,
        surface_width,
        surface_height,
        screen->format->BitsPerPixel,
        screen->format->Rmask,
        screen->format->Gmask,
        screen->format->Bmask,
        screen->format->Amask
    );

    SDL_Rect viewport_rect = {0, 0, screen->w, screen->h};
    seqview->viewport_rect = viewport_rect;
}

int seqview_render(seqview_t* seqview, SDL_Surface* screen) {

    // Only worry about visible portion of surface
    SDL_SetClipRect(seqview->surface, &seqview->viewport_rect);

    // Draw background
    seqview_fill_rect(
        seqview->surface,
        0,
        0,
        seqview->surface->w,
        seqview->surface->h,
        SDL_MapRGB(seqview->surface->format, SEQVIEW_BACKGROUND_COLOR_RGB)
    );

    // Draw steps
    track_t* track;
    step_t* step;
    int track_i;
    int step_i;
    for (track_i = 0; track_i < seqview->sequencer->num_tracks; track_i++) {
        track = &seqview->sequencer->tracks[track_i];
        for (step_i = 0; step_i < track->num_steps; step_i++) {
            step = &track->steps[step_i];
            seqview_fill_rect(
                seqview->surface,
                SEQVIEW_STEP_PADDING * (step_i + 1) + SEQVIEW_STEP_SIZE * step_i,
                SEQVIEW_STEP_PADDING * (track_i + 1) + SEQVIEW_STEP_SIZE * track_i,
                SEQVIEW_STEP_SIZE,
                SEQVIEW_STEP_SIZE,
                track->active_step == step
                    ? SDL_MapRGB(seqview->surface->format, SEQVIEW_STEP_COLOR_ACTIVE_RGB)
                    : SDL_MapRGB(seqview->surface->format, SEQVIEW_STEP_COLOR_INACTIVE_RGB)
            );
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
