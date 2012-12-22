#include <stdlib.h>
#include <stdbool.h>
#include <portmidi.h>

#include "track.h"
#include "sequencer.h"
#include "step.h"
#include "util.h"

int track_init(track_t* track, sequencer_t* sequencer) {
    track->sequencer = sequencer;
    track->steps = (step_t*)calloc(DEFAULT_MAX_STEPS_PER_TRACK, sizeof(step_t));
    track->active_step = NULL;
    track->max_steps = DEFAULT_MAX_STEPS_PER_TRACK;
    track->type = TYPE_NOTE;
    track->num_steps = DEFAULT_TRACK_LENGTH;
    int i;
    for (i = 0; i < track->max_steps; i++) {
        step_init(track, &track->steps[i]);
    }
    track->num_enabled_steps = track->num_steps;
    track->clock_divisor = 1;
    track->cc_number = 1;
    track->step_cursor = 0;
    track->step_counter = 0;
    track->step_delta = 1;
    track->step_offset = 0;
    track->step_roll_start = 0;
    track->step_roll_length = 1;
    track->note_velocity = DEFAULT_TRACK_NOTE_VELOCITY;
    track->note_duration = DEFAULT_TRACK_NOTE_DURATION;
    track->note_gate = DEFAULT_TRACK_NOTE_GATE;
    track->transpose_delta = 0;
    track->is_rolling = FALSE;
    track->is_muted = FALSE;
    track->midi_stream = NULL;
    track->midi_channel = 0;
    return 0;
}

int track_reset(track_t* track) {
    track->active_step = NULL;
    track->step_cursor = 0;
    track->step_counter = 0;
    return 0;
}

int track_set_length(track_t* self, int length) {
    length = MAX(1, MIN(self->max_steps, length));
    self->num_steps = length;
    return 0;
}

int track_set_step_enable(track_t* self, int step_index, bool enabled) {
    step_index = MAX(0, MIN(self->max_steps - 1, step_index));
    (self->steps + step_index)->is_enabled = enabled;
    self->num_enabled_steps += (enabled ? 1 : -1);
    return 0;
}

int track_on_clock_tick(track_t* track) {
    if (track->num_enabled_steps < 1) {
        return 0;
    }

    int step_cursor = track->step_counter + track->step_offset;
    if (track->is_rolling) {
        step_cursor = track->step_roll_start + (step_cursor % track->step_roll_length);
    }

    step_cursor = step_cursor % track->num_enabled_steps;
    int i;
    for (i = 0; i < track->num_steps; i++) {
        if (track->steps[i].is_enabled) {
            if (step_cursor > 0) {
                step_cursor -= 1;
            } else {
                track->step_cursor = i;
                break;
            }
        }
    }

    track->active_step = &track->steps[track->step_cursor];
    if (!track->is_muted
        && track->midi_stream
        && !track->active_step->is_muted
        && track->active_step->is_enabled
    ) {
        step_execute(track->active_step);
    }

    track->step_cursor = step_cursor;
    track->step_counter += track->step_delta;
    return 0;
}
