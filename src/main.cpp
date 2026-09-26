#include "nus_background_manager.h"

#include <coreinit/title.h>
#include <wups.h>
#include <wut.h>

#include <atomic>

namespace {
constexpr uint64_t kNUSspliTitleId = 0x00050000101C9500ULL;

std::atomic_bool gNUSspliActive{false};

bool IsNUSspli() {
    return OSGetTitleID() == kNUSspliTitleId;
}

void ResetRuntimeState() {
    gNUSspliActive.store(false);
    nusbg::ClearQueue();
}
}

WUPS_PLUGIN_NAME("NUSspli Background Download Manager");
WUPS_PLUGIN_DESCRIPTION("Queues NUSspli downloads for native Download Management after NUSspli exits");
WUPS_PLUGIN_VERSION("0.2.0");
WUPS_PLUGIN_AUTHOR("baldbuffalo");
WUPS_PLUGIN_LICENSE("GPL-3.0");

WUPS_USE_WUT_DEVOPTAB();

INITIALIZE_PLUGIN() {
    ResetRuntimeState();
}

ON_APPLICATION_START() {
    ResetRuntimeState();
    gNUSspliActive.store(IsNUSspli());
}

ON_APPLICATION_ENDS() {
    if (!gNUSspliActive.exchange(false)) {
        return;
    }

    nusbg::HandoffQueuedDownloads();
}
