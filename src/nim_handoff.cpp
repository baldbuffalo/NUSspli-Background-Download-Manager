#include "nus_background_manager.h"

#include <coreinit/debug.h>
#include <coreinit/dynload.h>
#include <cstdint>

namespace {

struct TitlePackageTaskConfig {
    uint32_t titleIdHigh;                 // 0x00
    uint32_t titleIdLow;                  // 0x04
    uint32_t regionOrLanguageRelated;     // 0x08
    uint8_t titleType;                    // 0x0C
    uint8_t applicationBoxDevice1;        // 0x0D
    uint8_t unknown0E;                    // 0x0E
    uint8_t applicationBoxDevice2;        // 0x0F
    uint32_t unknown10;                   // 0x10
    uint8_t unknown14;                    // 0x14
    uint8_t unknown15;                    // 0x15
    uint8_t postDownloadAction;           // 0x16
    uint8_t unknown17;                    // 0x17
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

    // This matches Cemu's nn_nim TitlePackageTaskConfig ABI:
    // 0x18 bytes, title ID at 0x00, title type at 0x0C,
    // device fields at 0x0D/0x0F, and post-download action at 0x16.
    static const uint16_t kTaskName[] = {
        'N','U','S','s','p','l','i',' ','D','o','w','n','l','o','a','d',0
    };

    for (const auto &task : queued) {
        if (task.title_id == 0) {
            continue;
        }

        TitlePackageTaskConfig config{};

        // Cemu's implementation of the native constructor receives:
        //   TitlePackageTaskConfig* output
        //   uint64_t titleId
        //   uint32_t region/language (currently ignored by Cemu)
        //   uint32_t TitleType
        //
        // Its implementation initializes the remaining fields itself,
        // including the MLC device fields and background-install policy.
        gMakeConfig(&config, task.title_id, 0, 0);

        const int32_t result = gRegisterTask(&config, kTaskName, 16);

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
