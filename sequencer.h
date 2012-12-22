#ifndef _SEQUENCER_H
#define _SEQUENCER_H

#include <stdlib.h>
#include <SDL/SDL_thread.h>
#include <SDL/SDL_mutex.h>

#include "track.h"

typedef enum {CLOCK_INTERNAL, CLOCK_EXTERNAL} sequencer_clock_t;

typedef enum {STATE_PLAY, STATE_PAUSE, STATE_STOP} sequencer_state_t;

typedef struct sequencer_s {
    sequencer_clock_t clock_type;
    float bpm;
    int delay_ms;
    sequencer_state_t state;
    SDL_mutex* state_lock;
    SDL_Thread* thread;
    struct track_s* tracks;
    int num_tracks;
    int max_steps_per_track;
    int ticks_per_beat;
    int default_track_length;
} sequencer_t;

int sequencer_loop(void* data);
sequencer_t* sequencer_new();
int sequencer_set_bpm(sequencer_t* sequencer, float bpm);
int sequencer_start(sequencer_t* sequencer);
int sequencer_pause(sequencer_t* sequencer);
int sequencer_stop(sequencer_t* sequencer);
int sequencer_schedule_note_off(sequencer_t* sequencer, int note, int duration_in_ticks);
int sequencer_reset_tracks(sequencer_t* sequencer);

#endif
