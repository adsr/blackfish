#include <stdlib.h>
#include <stdbool.h>
#include <portmidi.h>

#include "track.h"
#include "sequencer.h"
#include "step.h"
#include "util.h"
#include "config.h"

int track_init(track_t* track, sequencer_t* sequencer) {
    int i;
    track->sequencer = sequencer;
    track->steps = (step_t*)calloc(sequencer->max_steps_per_track, sizeof(step_t));
    track->active_step = NULL;
    track->type = TYPE_NOTE;
    track->num_steps = sequencer->default_track_length;
    for (i = 0; i < sequencer->max_steps_per_track; i++) {
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
    track->note_velocity = config_get("track.default_note_velocity", 100);
    track->note_duration = config_get("track.default_note_duration", 1);
    track->note_gate = (float)(config_get("track.default_note_gate", 16) / 16.0f);
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
    int step_i;
    int num_enabled_steps;

    length = MAX(1, MIN(self->sequencer->max_steps_per_track, length));
    self->num_steps = length;
    num_enabled_steps = 0;
    for (step_i = 0; step_i < self->num_steps; step_i++) {
        if (self->steps[step_i].is_enabled) {
            num_enabled_steps += 1;
        }
    }
    self->num_enabled_steps = num_enabled_steps;
    return 0;
}

int track_set_step_enable(track_t* self, int step_index, bool enabled) {
    step_index = MAX(0, MIN(self->sequencer->max_steps_per_track - 1, step_index));
    (self->steps + step_index)->is_enabled = enabled;
    self->num_enabled_steps += (enabled ? 1 : -1);
    return 0;
}

int track_on_clock_tick(track_t* track) {
    int step_cursor;
    int i;

    if (track->num_enabled_steps < 1) {
        return 0;
    }

    step_cursor = track->step_counter + track->step_offset;
    if (track->is_rolling) {
        step_cursor = track->step_roll_start + (step_cursor % track->step_roll_length);
    }

    step_cursor = step_cursor % track->num_enabled_steps;
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
