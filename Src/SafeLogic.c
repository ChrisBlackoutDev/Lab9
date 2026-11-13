/*
 * Breadboard safe interlock and request timing
 *
 * No GPIO is accessed here; the caller supplies the button and ADC readings.
 * The 25 ms button debounce follows the earlier Lab 3 lock program.
 */
#include "SafeLogic.h"

#define BUTTON_DEBOUNCE_MS 25U
#define POT_LOW_MAX 1228U
#define POT_HIGH_MIN 2867U

void InitializeSafe(SafeState *state, uint32_t now) {
    *state = (SafeState){0};
    state->door_candidate_since = now;
}

void CancelSafe(SafeState *state) {
    state->pulse_active = false;
}

void ExpireSafe(SafeState *state, uint32_t now, uint32_t pulse_ms) {
    if (state->pulse_active && (uint32_t)(now - state->pulse_started) >= pulse_ms) {
        CancelSafe(state);
    }
}

static bool RequestSafe(SafeState *state, uint32_t now) {
    if (!state->sensor_valid || !state->door_closed || !state->button_pressed ||
        state->pulse_active) {
        return false;
    }
    state->pulse_active = true;
    state->pulse_started = now;
    return true;
}

void SampleSafe(SafeState *state, uint32_t now, bool button_pressed,
                 uint16_t pot, uint16_t light, uint32_t pulse_ms) {
    state->pot = pot;
    state->light = light;
    state->sensor_valid = true;
    state->button_pressed = button_pressed;
    ExpireSafe(state, now, pulse_ms);
    if (!button_pressed) {
        /* A released interlock is unsafe immediately; only pressing is debounced. */
        state->door_closed = false;
        state->door_candidate = false;
        state->door_candidate_since = now;
        CancelSafe(state);
    } else if (button_pressed != state->door_candidate) {
        /* Restart the Lab 3 debounce timer when the raw button state changes. */
        state->door_candidate = button_pressed;
        state->door_candidate_since = now;
    } else if (state->door_closed != state->door_candidate &&
               (uint32_t)(now - state->door_candidate_since) >= BUTTON_DEBOUNCE_MS) {
        state->door_closed = state->door_candidate;
    }

    if (pot <= POT_LOW_MAX) {
        state->pot_armed = true;
    } else if (pot >= POT_HIGH_MIN && state->pot_armed) {
        state->pot_armed = false;
        (void)RequestSafe(state, now);
    }
}

bool ProcessSafeCommand(SafeState *state, char command, uint32_t now) {
    if (command == 'L' || command == 'l') {
        CancelSafe(state);
        return true;
    }
    if (command == 'U' || command == 'u') {
        return RequestSafe(state, now);
    }
    return false;
}
