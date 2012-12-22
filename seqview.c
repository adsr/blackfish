#include <stdlib.h>
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>

#include "seqview.h"
#include "sequencer.h"
#include "config.h"

seqview_t* seqview_new(SDL_Surface* screen, sequencer_t* sequencer) {

    seqview_t* seqview = (seqview_t*)calloc(1, sizeof(seqview_t));

    seqview->sequencer = sequencer;

    seqview->step_size = config_get("seqview.step_size", 32);
    seqview->step_padding = config_get("seqview.step_padding", 4);

    int surface_width = seqview->step_padding * (sequencer->max_steps_per_track + 1)
        + (seqview->step_size * sequencer->max_steps_per_track);
    int surface_height = seqview->step_padding * (sequencer->default_track_length + 1)
        + (seqview->step_size * sequencer->default_track_length);

    // Create surface
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

    seqview->color_step_active = SDL_MapRGB(seqview->surface->format, SEQVIEW_COLOR_STEP_ACTIVE);
    seqview->color_step_inactive = SDL_MapRGB(seqview->surface->format, SEQVIEW_COLOR_STEP_INACTIVE);
    seqview->color_background = SDL_MapRGB(seqview->surface->format, SEQVIEW_COLOR_BACKGROUND);

    SDL_Rect viewport_rect = {0, 0, screen->w, screen->h};
    seqview->viewport_rect = viewport_rect;

    return seqview;
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
        seqview->color_background
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
                seqview->step_padding * (step_i + 1) + seqview->step_size * step_i,
                seqview->step_padding * (track_i + 1) + seqview->step_size * track_i,
                seqview->step_size,
                seqview->step_size,
                track->active_step == step
                    ? seqview->color_step_active
                    : seqview->color_step_inactive
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
