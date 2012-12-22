#ifndef _TRACK_H
#define _TRACK_H

#include <stdlib.h>
#include <stdbool.h>
#include <portmidi.h>

#include "sequencer.h"
#include "step.h"

#define DEFAULT_MAX_STEPS_PER_TRACK 256
#define DEFAULT_TRACK_LENGTH 16
#define DEFAULT_TRACK_NOTE_VELOCITY 100
#define DEFAULT_TRACK_NOTE_DURATION 1
#define DEFAULT_TRACK_NOTE_GATE 1.0f

typedef enum {TYPE_NOTE, TYPE_CONTROL} track_type_t;

typedef struct track_s {
    struct sequencer_s* sequencer;
    struct step_s* steps;
    struct step_s* active_step;
    int max_steps;
    track_type_t type;
    int num_steps;
    int num_enabled_steps;
    int clock_divisor;
    int cc_number;
    int step_cursor;
    int step_counter;
    int step_delta;
    int step_offset;
    int step_roll_start;
    int step_roll_length;
    int note_velocity;
    int note_duration;
    float note_gate;
    int transpose_delta;
    bool is_rolling;
    bool is_muted;
    //bool is_forcing_scale;
    PortMidiStream* midi_stream;
    int midi_channel;
} track_t;

int track_init(track_t* track, struct sequencer_s* sequencer);
int track_set_length(track_t* self, int length);
int track_set_step_enable(track_t* self, int step_index, bool enabled);
int track_on_clock_tick(track_t* track);
int track_reset(track_t* track);

#endif
