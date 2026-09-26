#include "nus_background_manager.h"

#include <coreinit/title.h>
#include <wups.h>
#include <wut.h>

#include <atomic>

namespace {

constexpr uint64_t kNUSspliTitleIdEUR = 0x00050000101C9500ULL;
constexpr uint64_t kNUSspliTitleIdUSA = 0x00050000101C9400ULL;
constexpr uint64_t kNUSspliTitleIdJPN = 0x00050000101C9300ULL;

std::atomic_bool gNUSspliActive{false};

bool IsNUSspli() {
    const uint64_t titleId = OSGetTitleID();
    return titleId == kNUSspliTitleIdEUR ||
           titleId == kNUSspliTitleIdUSA ||
           titleId == kNUSspliTitleIdJPN;
}

void ResetRuntimeState() {
    gNUSspliActive.store(false);
    nusbg::ClearQueue();
}

} // namespace

WUPS_PLUGIN_NAME("NUSspli Background Download Manager");
WUPS_PLUGIN_DESCRIPTION("Queues NUSspli downloads for native Download Management after NUSspli exits");
WUPS_PLUGIN_VERSION("0.3.0");
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

    // This is the last application lifecycle callback. NUSspli has stopped
    // its normal downloader before this handoff is performed.
    nusbg::HandoffQueuedDownloads();
}
