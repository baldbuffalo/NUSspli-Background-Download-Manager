#include "nus_background_manager.h"

#include <coreinit/dynload.h>
#include <coreinit/debug.h>

#include <cstdint>
#include <vector>

namespace {

using MakeConfigFn = int32_t (*)(void *config, uint64_t titleId, uint32_t regionOrLanguage, uint32_t titleType);
using RegisterTaskFn = int32_t (*)(const void *config, const uint16_t *contentIds, uint32_t contentCount);

constexpr const char *kNimRpl = "nn_nim.rpl";
constexpr const char *kMakeConfig =
    "MakeTitlePackageTaskConfigAutoUsingBgInstallPolicy__Q3_2nn3nim4utilFULiQ3_2nn4Cafe9TitleType";
constexpr const char *kRegisterTask =
    "RegisterTitlePackageTask__Q2_2nn3nimFRCQ3_2nn3nim22TitlePackageTaskConfigPCUsUi";

struct TitlePackageTaskConfig {
    uint32_t titleIdHigh;
    uint32_t titleIdLow;
    uint32_t regionOrLanguageRelated;
    uint8_t unknown0C;
    uint8_t applicationBoxDevice1;
    uint8_t unknown0E;
    uint8_t applicationBoxDevice2;
    uint32_t unknown10;
    uint8_t unknown14;
    uint8_t unknown15;
    uint8_t postDownloadAction;
    uint8_t unknown17;
};

static_assert(sizeof(TitlePackageTaskConfig) == 0x18, "NIM config must be 0x18 bytes");

bool ResolveNim(MakeConfigFn &makeConfig, RegisterTaskFn &registerTask) {
    OSDynLoad_Module module = nullptr;

    if (OSDynLoad_Acquire(kNimRpl, &module) != OS_DYNLOAD_OK) {
        OSReport("[NUSBG] OSDynLoad_Acquire(nn_nim.rpl) failed\n");
        return false;
    }

    if (OSDynLoad_FindExport(module, OS_DYNLOAD_EXPORT_FUNC, kMakeConfig,
                             reinterpret_cast<void **>(&makeConfig)) != OS_DYNLOAD_OK) {
        OSReport("[NUSBG] NIM config factory export not found\n");
        return false;
    }

    if (OSDynLoad_FindExport(module, OS_DYNLOAD_EXPORT_FUNC, kRegisterTask,
                             reinterpret_cast<void **>(&registerTask)) != OS_DYNLOAD_OK) {
        OSReport("[NUSBG] NIM RegisterTitlePackageTask export not found\n");
        return false;
    }

    return true;
}

uint32_t TitleTypeFromTitleId(uint64_t titleId) {
    // Cafe::TitleType is passed as a 32-bit enum. The neutral value keeps
    // this adapter independent of SDK enum spelling; NIM still receives the
    // complete title ID and applies its package policy.
    (void) titleId;
    return 0;
}

} // namespace

namespace nusbg {

void HandoffQueuedDownloads() {
    const std::vector<DownloadTask> tasks = SnapshotQueue();

    if (tasks.empty()) {
        return;
    }

    MakeConfigFn makeConfig = nullptr;
    RegisterTaskFn registerTask = nullptr;

    if (!ResolveNim(makeConfig, registerTask)) {
        OSReport("[NUSBG] Native NIM handoff unavailable\n");
        return;
    }

    std::vector<DownloadTask> failed;
    failed.reserve(tasks.size());

    for (const auto &task : tasks) {
        TitlePackageTaskConfig config{};

        const int32_t makeResult =
            makeConfig(&config,
                       task.title_id,
                       static_cast<uint32_t>(task.region),
                       TitleTypeFromTitleId(task.title_id));

        if (makeResult != 0) {
            OSReport("[NUSBG] MakeTitlePackageTaskConfig failed for %016llX: %d\n",
                     static_cast<unsigned long long>(task.title_id),
                     makeResult);
            failed.push_back(task);
            continue;
        }

        // RegisterTitlePackageTask creates the native title-package task.
        // No explicit content list is supplied; NIM obtains package metadata
        // from its own title/package services.
        const int32_t registerResult = registerTask(&config, nullptr, 0);

        if (registerResult != 0) {
            OSReport("[NUSBG] RegisterTitlePackageTask failed for %016llX: %d\n",
                     static_cast<unsigned long long>(task.title_id),
                     registerResult);
            failed.push_back(task);
            continue;
        }

        OSReport("[NUSBG] Native task registered for %016llX\n",
                 static_cast<unsigned long long>(task.title_id));
    }

    ReplaceQueue(failed);
}

} // namespace nusbg
