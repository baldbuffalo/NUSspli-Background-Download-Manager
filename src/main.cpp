#include "nus_background_manager.h"

#include <coreinit/title.h>
#include <wups.h>
#include <wups/config.h>
#include <coreinit/debug.h>
#include <wut.h>
#include <function_patcher/function_patching.h>

#include <atomic>

extern bool InstallNUSspliHook();

namespace {

constexpr uint64_t kNUSspliTitleId = 0x0005000010155373ULL;

std::atomic_bool gNUSspliActive{false};

bool IsNUSspli() {
    return OSGetTitleID() == kNUSspliTitleId;
}

} // namespace

WUPS_PLUGIN_NAME("NUSspli Background Download Manager");
WUPS_PLUGIN_DESCRIPTION("Queues NUSspli downloads and hands them to native Download Management after NUSspli exits");
WUPS_PLUGIN_VERSION("0.5.0");
WUPS_PLUGIN_AUTHOR("baldbuffalo");
WUPS_PLUGIN_LICENSE("GPL-3.0");


INITIALIZE_PLUGIN() {
    gNUSspliActive.store(false);
    const auto status = FunctionPatcher_InitLibrary();
    if (status != FUNCTION_PATCHER_RESULT_SUCCESS) {
        OSReport("[NUSBG] FunctionPatcherModule unavailable: %d\n", status);
    } else {
        InstallNUSspliHook();
    }
}

ON_APPLICATION_START() {
    gNUSspliActive.store(IsNUSspli());
}

ON_APPLICATION_ENDS() {
    if (!gNUSspliActive.exchange(false)) {
        return;
    }

    // NUSspli's actual application-end hook is the handoff point. We do not
    // start native tasks while NUSspli is still running.
    nusbg::HandoffQueuedDownloads();
}
