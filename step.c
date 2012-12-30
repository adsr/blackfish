#include <stdlib.h>
#include <stdbool.h>
#include <portmidi.h>

#include "step.h"
#include "sequencer.h"
#include "track.h"

void step_init(track_t* track, step_t* step) {
    int i;
    step->track = track;
    step->notes = (int*)calloc(DEFAULT_MAX_NOTES_PER_STEP, sizeof(int));
    for (i = 0; i < DEFAULT_MAX_NOTES_PER_STEP; i++) {
        step->notes[i] = NOTE_NONE;
    }
    step->num_notes = DEFAULT_MAX_NOTES_PER_STEP;
    step->note_velocity = NOTE_VELOCITY_NONE;
    step->note_duration = NOTE_DURATION_NONE;
    step->note_gate = NOTE_GATE_NONE;
    step->cc_value = 0;
    step->is_muted = FALSE;
    step->is_enabled = TRUE;
    step->has_transpose_defeat = FALSE;
}

void step_execute(step_t* step) {
    int i;
    int note;
    if (step->track->type == TYPE_NOTE) {
        for (i = 0; i < step->num_notes; i++) {
            if (step->notes[i] != NOTE_NONE) {
                note = (step->notes[i] + (step->has_transpose_defeat ? 0 : step->track->transpose_delta)) & 0x7f;
                Pm_WriteShort(
                    step->track->midi_stream,
                    0L,
                    Pm_Message(
                        MIDI_STATUS_NOTE_ON,
                        note,
                        step->note_velocity == NOTE_VELOCITY_NONE ? step->track->note_velocity : step->note_velocity
                    )
                );
                sequencer_schedule_note_off(
                    step->track->sequencer,
                    note,
                    (int)(
                        (step->note_duration == NOTE_DURATION_NONE ? step->track->note_duration : step->note_duration)
                        * (step->note_gate == NOTE_GATE_NONE ? step->track->note_gate : step->note_gate)
                    )
                );
            }
        }
    } else {
        Pm_WriteShort(
            step->track->midi_stream,
            0L,
            Pm_Message(
                MIDI_STATUS_CC,
                step->track->cc_number + step->track->transpose_delta,
                step->cc_value
            )
        );
    }
}
