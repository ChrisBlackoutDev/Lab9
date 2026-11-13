#include <assert.h>
#include "SafeLogic.h"

int main(void) {
    SafeState state = {0};
    InitializeSafe(&state, 0);
    assert(!ProcessSafeCommand(&state, 'U', 0)); /* No sensor sample yet. */
    SampleSafe(&state, 0, false, 4095, 0, 100);
    assert(!state.pulse_active); /* Boot high cannot unlock. */

    SampleSafe(&state, 10, true, 0, 0, 100);
    SampleSafe(&state, 40, true, 0, 0, 100);
    assert(state.door_closed && state.pot_armed);
    SampleSafe(&state, 60, true, 4095, 0, 100);
    assert(state.pulse_active);
    SampleSafe(&state, 80, true, 4095, 0, 100);
    assert(state.pulse_active); /* Holding high does not retrigger. */
    ExpireSafe(&state, 160, 100);
    assert(!state.pulse_active);
    SampleSafe(&state, 180, true, 4095, 0, 100);
    assert(!state.pulse_active);

    assert(ProcessSafeCommand(&state, 'U', 190));
    assert(state.pulse_active);
    SampleSafe(&state, 200, false, 4095, 0, 100);
    assert(!state.pulse_active);
    assert(!ProcessSafeCommand(&state, 'U', 201));
    SampleSafe(&state, 230, false, 4095, 0, 100);
    assert(!state.door_closed && !state.pulse_active);
    assert(!ProcessSafeCommand(&state, 'U', 240));

    /* A high pot while open is consumed, so closing later cannot surprise-unlock. */
    SampleSafe(&state, 245, false, 0, 0, 100);
    SampleSafe(&state, 246, false, 4095, 0, 100);
    SampleSafe(&state, 250, true, 4095, 0, 100);
    SampleSafe(&state, 280, true, 4095, 0, 100);
    assert(!state.pulse_active);

    SampleSafe(&state, 281, true, 0, 0, 100);
    assert(ProcessSafeCommand(&state, 'U', 290));
    assert(ProcessSafeCommand(&state, 'L', 291));
    assert(!state.pulse_active);
    return 0;
}
