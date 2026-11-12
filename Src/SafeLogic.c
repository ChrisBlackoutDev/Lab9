#include "SafeLogic.h"

#define BUTTON_DEBOUNCE_MS 25U
#define POT_LOW_MAX 1228U
#define POT_HIGH_MIN 2867U

void safe_init(SafeState *state, uint32_t now) {
    *state = (SafeState){0};
    state->door_candidate_since = now;
}

void safe_cancel(SafeState *state) {
    state->pulse_active = false;
}

void safe_expire(SafeState *state, uint32_t now, uint32_t pulse_ms) {
    if (state->pulse_active && (uint32_t)(now - state->pulse_started) >= pulse_ms) {
        safe_cancel(state);
    }
}

static bool safe_request(SafeState *state, uint32_t now) {
    if (!state->sensor_valid || !state->door_closed || !state->button_pressed ||
        state->pulse_active) {
        return false;
    }
    state->pulse_active = true;
    state->pulse_started = now;
    return true;
}

void safe_sample(SafeState *state, uint32_t now, bool button_pressed,
                 uint16_t pot, uint16_t light, uint32_t pulse_ms) {
    state->pot = pot;
    state->light = light;
    state->sensor_valid = true;
    state->button_pressed = button_pressed;
    safe_expire(state, now, pulse_ms);
    if (!button_pressed) {
        /* A released interlock is unsafe immediately; only pressing is debounced. */
        state->door_closed = false;
        state->door_candidate = false;
        state->door_candidate_since = now;
        safe_cancel(state);
    } else if (button_pressed != state->door_candidate) {
        /* Adapted from the user's Lab 3 nonblocking pushbutton debounce. */
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
        (void)safe_request(state, now);
    }
}

bool safe_command(SafeState *state, char command, uint32_t now) {
    if (command == 'L' || command == 'l') {
        safe_cancel(state);
        return true;
    }
    if (command == 'U' || command == 'u') {
        return safe_request(state, now);
    }
    return false;
}
