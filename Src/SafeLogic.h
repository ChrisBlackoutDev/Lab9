#ifndef SAFE_LOGIC_H_
#define SAFE_LOGIC_H_

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    bool door_closed;
    bool door_candidate;
    bool button_pressed;
    bool pot_armed;
    bool pulse_active;
    bool sensor_valid;
    uint32_t door_candidate_since;
    uint32_t pulse_started;
    uint16_t pot;
    uint16_t light;
} SafeState;

void InitializeSafe(SafeState *state, uint32_t now);
void SampleSafe(SafeState *state, uint32_t now, bool button_pressed,
                 uint16_t pot, uint16_t light, uint32_t pulse_ms);
bool ProcessSafeCommand(SafeState *state, char command, uint32_t now);
void ExpireSafe(SafeState *state, uint32_t now, uint32_t pulse_ms);
void CancelSafe(SafeState *state);

#endif
