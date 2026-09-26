#include "nus_background_manager.h"

#include <coreinit/debug.h>

namespace nusbg {

void HandoffQueuedDownloads() {
    // Disabled until the exact retail nn_nim ABI for title-package task
    // registration is verified. Calling the previously guessed ABI caused
    // a freeze when NUSspli ended.
    OSReport("[NUSBG] NIM handoff disabled: retail ABI not verified\\n");
}

} // namespace nusbg
