#include <assert.h>
#include "SafeLogic.h"

int main(void) {
    SafeState state;
    safe_init(&state, 0);
    assert(!safe_command(&state, 'U', 0)); /* No sensor sample yet. */
    safe_sample(&state, 0, false, 4095, 0, 100);
    assert(!state.pulse_active); /* Boot high cannot unlock. */

    safe_sample(&state, 10, true, 0, 0, 100);
    safe_sample(&state, 40, true, 0, 0, 100);
    assert(state.door_closed && state.pot_armed);
    safe_sample(&state, 60, true, 4095, 0, 100);
    assert(state.pulse_active);
    safe_sample(&state, 80, true, 4095, 0, 100);
    assert(state.pulse_active); /* Holding high does not retrigger. */
    safe_expire(&state, 160, 100);
    assert(!state.pulse_active);
    safe_sample(&state, 180, true, 4095, 0, 100);
    assert(!state.pulse_active);

    assert(safe_command(&state, 'U', 190));
    assert(state.pulse_active);
    safe_sample(&state, 200, false, 4095, 0, 100);
    assert(!state.pulse_active);
    assert(!safe_command(&state, 'U', 201));
    safe_sample(&state, 230, false, 4095, 0, 100);
    assert(!state.door_closed && !state.pulse_active);
    assert(!safe_command(&state, 'U', 240));

    /* A high pot while open is consumed, so closing later cannot surprise-unlock. */
    safe_sample(&state, 245, false, 0, 0, 100);
    safe_sample(&state, 246, false, 4095, 0, 100);
    safe_sample(&state, 250, true, 4095, 0, 100);
    safe_sample(&state, 280, true, 4095, 0, 100);
    assert(!state.pulse_active);

    safe_sample(&state, 281, true, 0, 0, 100);
    assert(safe_command(&state, 'U', 290));
    assert(safe_command(&state, 'L', 291));
    assert(!state.pulse_active);
    return 0;
}
