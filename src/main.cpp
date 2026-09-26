#include "nus_background_manager.h"

#include <wups.h>
#include <wut.h>

namespace {
bool gNUSspliActive = false;

void ResetRuntimeState() {
    gNUSspliActive = false;
    nusbg::ClearQueue();
}
}

WUPS_PLUGIN_NAME("NUSspli Background Download Manager");
WUPS_PLUGIN_DESCRIPTION("Queues NUSspli downloads for native Download Management after NUSspli exits");
WUPS_PLUGIN_VERSION("0.1.0");
WUPS_PLUGIN_AUTHOR("baldbuffalo");
WUPS_PLUGIN_LICENSE("GPL-3.0");

WUPS_USE_WUT_DEVOPTAB();

INITIALIZE_PLUGIN() {
    ResetRuntimeState();
}

ON_APPLICATION_START() {
    ResetRuntimeState();
}

ON_APPLICATION_ENDS() {
    if (!gNUSspliActive) {
        return;
    }

    nusbg::HandoffQueuedDownloads();
    gNUSspliActive = false;
}
