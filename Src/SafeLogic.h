#ifndef SAFE_LOGIC_H_
#define SAFE_LOGIC_H_

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    bool unlock_requested;
    bool button_raw;
    bool button_pressed;
    bool button_initialized;
    bool pot_armed;
    uint32_t button_changed_at;
    uint16_t pot;
    uint16_t light;
} SafeState;

void InitializeSafe(SafeState *state, uint32_t now);
void SampleSafe(SafeState *state, uint32_t now, bool button_pressed,
                uint16_t pot, uint16_t light);
bool ProcessSafeCommand(SafeState *state, char command);
void LockSafe(SafeState *state);

#endif
