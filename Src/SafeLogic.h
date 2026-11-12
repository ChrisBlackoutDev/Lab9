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

void safe_init(SafeState *state, uint32_t now);
void safe_sample(SafeState *state, uint32_t now, bool button_pressed,
                 uint16_t pot, uint16_t light, uint32_t pulse_ms);
bool safe_command(SafeState *state, char command, uint32_t now);
void safe_expire(SafeState *state, uint32_t now, uint32_t pulse_ms);
void safe_cancel(SafeState *state);

#endif
