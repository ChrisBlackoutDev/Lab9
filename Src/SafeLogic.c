/*
 * Breadboard lock request logic; the caller supplies button and ADC readings.
 */
#include "SafeLogic.h"

#define BUTTON_DEBOUNCE_MS 25U
#define POT_LOW_MAX 1228U
#define POT_HIGH_MIN 2867U

void InitializeSafe(SafeState *state, uint32_t now) {
    *state = (SafeState){0};
    state->button_changed_at = now;
}

void LockSafe(SafeState *state) {
    state->unlock_requested = false;
}

void SampleSafe(SafeState *state, uint32_t now, bool button_pressed, uint16_t pot) {
    bool toggle = false;
    state->pot = pot;

    if (!state->button_initialized) {
        /* An already-held button at boot is not a new press. */
        state->button_raw = button_pressed;
        state->button_pressed = button_pressed;
        state->button_initialized = true;
    } else if (button_pressed != state->button_raw) {
        state->button_raw = button_pressed;
        state->button_changed_at = now;
    } else if (state->button_pressed != state->button_raw &&
               (uint32_t)(now - state->button_changed_at) >= BUTTON_DEBOUNCE_MS) {
        state->button_pressed = state->button_raw;
        toggle = state->button_pressed;
    }

    if (pot <= POT_LOW_MAX) {
        state->pot_armed = true;
    } else if (pot >= POT_HIGH_MIN && state->pot_armed) {
        state->pot_armed = false;
        toggle = true;
    }

    if (toggle) {
        state->unlock_requested = !state->unlock_requested;
    }
}

bool ProcessSafeCommand(SafeState *state, char command) {
    if (command == 'U' || command == 'u') {
        state->unlock_requested = !state->unlock_requested;
        return true;
    }
    if (command == 'L' || command == 'l') {
        LockSafe(state);
        return true;
    }
    return false;
}
