#ifndef _STEP_H
#define _STEP_H

#include <stdlib.h>
#include <stdbool.h>

#include "sequencer.h"
#include "track.h"

#define DEFAULT_MAX_NOTES_PER_STEP 4

#define MIDI_STATUS_NOTE_ON 0x90
#define MIDI_STATUS_CC 0xb0

#define NOTE_NONE -1
#define NOTE_VELOCITY_NONE -1
#define NOTE_GATE_NONE -1.0f
#define NOTE_DURATION_NONE -1

typedef struct step_s {
    struct track_s* track;
    int* notes;
    int num_notes;
    int note_velocity;
    int note_duration;
    float note_gate;
    int cc_value;
    bool is_muted;
    bool is_enabled;
    bool has_transpose_defeat;
    //bool has_force_to_scale_defeat;
} step_t;

void step_init(struct track_s* track, step_t* step);
void step_execute(step_t* step);

#endif
