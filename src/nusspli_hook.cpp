#include <wups.h>
#include <coreinit/thread.h>

extern "C" void NUSBG_PollFromNUSspli();

DECL_FUNCTION(void, OSYieldThread) {
    NUSBG_PollFromNUSspli();
    real_OSYieldThread();
}

WUPS_MUST_REPLACE_FOR_PROCESS(
    OSYieldThread,
    WUPS_LOADER_LIBRARY_COREINIT,
    OSYieldThread,
    WUPS_FP_TARGET_PROCESS_ROOT_RPX
);
