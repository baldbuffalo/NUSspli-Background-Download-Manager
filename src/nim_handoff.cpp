#include "nus_background_manager.h"

#include <coreinit/debug.h>
#include <coreinit/dynload.h>
#include <cstdint>
#include <cstring>

namespace {

struct TitlePackageTaskConfig {
    uint32_t title_id_high;
    uint32_t title_id_low;
    uint32_t region_or_language;
    uint8_t title_type;
    uint8_t device1;
    uint8_t unknown_0e;
    uint8_t device2;
    uint32_t unknown_10;
    uint8_t unknown_14;
    uint8_t unknown_15;
    uint8_t post_download_action;
    uint8_t unknown_17;
};

static_assert(sizeof(TitlePackageTaskConfig) == 0x18, "TitlePackageTaskConfig must be 0x18 bytes");

using MakeConfigFn = void (*)(TitlePackageTaskConfig *, uint64_t, uint32_t, uint32_t);
using RegisterTaskFn = int32_t (*)(const TitlePackageTaskConfig *, const uint16_t *, uint32_t);

constexpr const char *kNimRpl = "nn_nim.rpl";
constexpr const char *kMakeConfigSymbol =
    "MakeTitlePackageTaskConfigAutoUsingBgInstallPolicy__Q3_2nn3nim4utilFULiQ3_2nn4Cafe9TitleType";
constexpr const char *kRegisterTaskSymbol =
    "RegisterTitlePackageTask__Q2_2nn3nimFRCQ3_2nn3nim22TitlePackageTaskConfigPCUsUi";

MakeConfigFn gMakeConfig = nullptr;
RegisterTaskFn gRegisterTask = nullptr;

bool ResolveNim() {
    if (gMakeConfig && gRegisterTask) {
        return true;
    }

    OSDynLoad_Module module = nullptr;
    if (OSDynLoad_Acquire(kNimRpl, &module) != 0) {
        OSReport("[NUSBG] Could not acquire nn_nim.rpl\\n");
        return false;
    }

    void *makeAddress = nullptr;
    void *registerAddress = nullptr;

    if (OSDynLoad_FindExport(module, OS_DYNLOAD_EXPORT_FUNC, kMakeConfigSymbol, &makeAddress) != 0 ||
        OSDynLoad_FindExport(module, OS_DYNLOAD_EXPORT_FUNC, kRegisterTaskSymbol, &registerAddress) != 0) {
        OSReport("[NUSBG] Could not resolve native NIM task APIs\\n");
        return false;
    }

    gMakeConfig = reinterpret_cast<MakeConfigFn>(makeAddress);
    gRegisterTask = reinterpret_cast<RegisterTaskFn>(registerAddress);
    return true;
}

} // namespace

namespace nusbg {

void HandoffQueuedDownloads() {
    const auto queued = SnapshotQueue();
    if (queued.empty()) {
        OSReport("[NUSBG] No queued downloads to hand off\\n");
        return;
    }

    if (!ResolveNim()) {
        OSReport("[NUSBG] NIM handoff postponed: APIs unavailable\\n");
        return;
    }

    // The native task API expects a UTF-16 display name and its character count.
    static const uint16_t kTaskName[] = {
        'N','U','S','s','p','l','i',' ','D','o','w','n','l','o','a','d',0
    };

    for (const auto &task : queued) {
        if (task.title_id == 0) {
            continue;
        }

        TitlePackageTaskConfig config{};
        // TitleType 0 is the normal/base Wii U title type.
        gMakeConfig(&config, task.title_id, task.region, 0);

        const int32_t result =
            gRegisterTask(&config, kTaskName, 16);

        if (result == 0) {
            RemoveDownload(task.task_id);
            OSReport("[NUSBG] Native Download Management task registered: %016llX\\n",
                     static_cast<unsigned long long>(task.title_id));
        } else {
            OSReport("[NUSBG] Native Download Management rejected %016llX: %d\\n",
                     static_cast<unsigned long long>(task.title_id),
                     static_cast<int>(result));
        }
    }
}

} // namespace nusbg
