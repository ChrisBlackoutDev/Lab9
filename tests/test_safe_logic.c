#include <assert.h>

#include "SafeLogic.h"

int main(void) {
    SafeState state = {0};
    InitializeSafe(&state, 0);
    assert(!state.unlock_requested);

    /* Inputs already held or high at boot do not request an unlock. */
    SampleSafe(&state, 20, true, 4095, 100);
    SampleSafe(&state, 60, true, 4095, 100);
    assert(!state.unlock_requested);
    assert(state.light == 100);

    SampleSafe(&state, 80, false, 4095, 100);
    SampleSafe(&state, 120, false, 4095, 100);
    SampleSafe(&state, 140, true, 4095, 100);
    SampleSafe(&state, 180, true, 4095, 100);
    assert(state.unlock_requested);
    SampleSafe(&state, 220, true, 4095, 100);
    assert(state.unlock_requested);
    SampleSafe(&state, 240, false, 4095, 100);
    SampleSafe(&state, 280, false, 4095, 100);
    SampleSafe(&state, 300, true, 4095, 100);
    SampleSafe(&state, 340, true, 4095, 100);
    assert(!state.unlock_requested);

    SampleSafe(&state, 360, true, 0, 100);
    SampleSafe(&state, 380, true, 4095, 100);
    assert(state.unlock_requested);
    SampleSafe(&state, 400, true, 4095, 100);
    assert(state.unlock_requested);
    SampleSafe(&state, 420, true, 0, 100);
    SampleSafe(&state, 440, true, 4095, 100);
    assert(!state.unlock_requested);

    assert(ProcessSafeCommand(&state, 'U'));
    assert(state.unlock_requested);
    assert(ProcessSafeCommand(&state, 'u'));
    assert(!state.unlock_requested);
    assert(ProcessSafeCommand(&state, 'U'));
    assert(ProcessSafeCommand(&state, 'L'));
    assert(!state.unlock_requested);
    assert(!ProcessSafeCommand(&state, '?'));

    /* A pot turn and button press in one sample produce one toggle. */
    SampleSafe(&state, 460, false, 0, 100);
    SampleSafe(&state, 500, false, 0, 100);
    SampleSafe(&state, 520, true, 0, 100);
    assert(!state.unlock_requested);
    SampleSafe(&state, 560, true, 4095, 100);
    assert(state.unlock_requested);

    LockSafe(&state);
    assert(!state.unlock_requested);
    return 0;
}
