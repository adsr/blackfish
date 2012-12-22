#include <stdlib.h>
#include <SDL/SDL.h>
#include <SDL/SDL_thread.h>

#include "sequencer.h"
#include "track.h"

sequencer_t* sequencer_new() {
    int num_tracks = DEFAULT_NUM_TRACKS; // TODO make configurable
    sequencer_t* sequencer = (sequencer_t*)calloc(1, sizeof(sequencer_t));
    sequencer->clock_type = CLOCK_INTERNAL;
    sequencer_set_bpm(sequencer, DEFAULT_BPM);
    sequencer->state = STATE_STOP;
    sequencer->tracks = (track_t*)calloc(num_tracks, sizeof(track_t));
    int i;
    for (i = 0; i < num_tracks; i++) {
        track_init(&sequencer->tracks[i], sequencer);
    }
    sequencer->num_tracks = num_tracks;
    return sequencer;
}

int sequencer_loop(void* data) {
    sequencer_t* sequencer = (sequencer_t*)data;
    // Enter sequencer loop
    Uint32 tick_start;
    Uint32 delay_time;
    int i;
    while (1) {
        // Remember start of tick
        tick_start = SDL_GetTicks();

        // Acquire state lock
        SDL_mutexP(sequencer->state_lock);
        if (sequencer->state != STATE_PLAY) {
            break;
        }

        // Raise clock tick on each track
        for (i = 0; i < sequencer->num_tracks; i++) {
            track_on_clock_tick(&sequencer->tracks[i]);
        }

        // Release state lock
        SDL_mutexV(sequencer->state_lock);

        // Wait for next tick
        delay_time = sequencer->delay_ms - (SDL_GetTicks() - tick_start);
        if (delay_time > 0) {
            SDL_Delay(delay_time);
        }
    }
    return 0;
}

int sequencer_set_bpm(sequencer_t* sequencer, float bpm) {
    if (bpm < 0.0f) {
        return 1;
    }
    sequencer->bpm = bpm;
    sequencer->delay_ms = (int)((60.0f / bpm) * (1000 / DEFAULT_TICKS_PER_BEAT));
    return 0;
}

int sequencer_start(sequencer_t* sequencer) {
    SDL_mutexP(sequencer->state_lock);
    if (sequencer->state != STATE_PLAY) {
        sequencer->state = STATE_PLAY;
        sequencer->thread = SDL_CreateThread(sequencer_loop, sequencer);
    }
    SDL_mutexV(sequencer->state_lock);
    return 0;
}

int sequencer_pause(sequencer_t* sequencer) {
    int status = 0;
    SDL_mutexP(sequencer->state_lock);
    if (sequencer->state == STATE_PLAY) {
        sequencer->state = STATE_PAUSE;
        SDL_WaitThread(sequencer->thread, &status);
    }
    SDL_mutexV(sequencer->state_lock);
    return status;
}

int sequencer_stop(sequencer_t* sequencer) {
    int status = 0;
    SDL_mutexP(sequencer->state_lock);
    if (sequencer->state == STATE_PLAY) {
        sequencer->state = STATE_STOP;
        SDL_WaitThread(sequencer->thread, &status);
        sequencer_reset_tracks(sequencer);
    } else if (sequencer->state == STATE_PAUSE) {
        sequencer->state = STATE_STOP;
        sequencer_reset_tracks(sequencer);
    }
    SDL_mutexV(sequencer->state_lock);
    return status;
}

int sequencer_schedule_note_off(sequencer_t* sequencer, int note, int duration_in_ticks) {
    return 0;
}

int sequencer_reset_tracks(sequencer_t* sequencer) {
    int i;
    for (i = 0; i < sequencer->num_tracks; i++) {
        track_reset(&sequencer->tracks[i]);
    }
    return 0;
}
