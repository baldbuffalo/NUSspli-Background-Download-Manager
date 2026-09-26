#include "nus_background_manager.h"

#include <function_patcher/function_patching.h>
#include <coreinit/debug.h>

#include <cstddef>
#include <cstdint>
#include <cstring>

namespace {

constexpr uint64_t kNUSspliTitleId = 0x0005000010155373ULL;
const uint64_t kTargetTitleIds[] = { kNUSspliTitleId };
uint64_t gNextTaskId = 1;
PatchedFunctionHandle gDownloadTitlePatch = 0;

uint64_t ReadTitleId(const void *tmd) {
    if (!tmd) return 0;
    uint64_t titleId = 0;
    std::memcpy(&titleId, static_cast<const uint8_t *>(tmd) + 0x18C, sizeof(titleId));
    return titleId;
}

uint16_t ReadTitleVersion(const void *tmd) {
    if (!tmd) return 0;
    uint16_t version = 0;
    std::memcpy(&version, static_cast<const uint8_t *>(tmd) + 0x1DC, sizeof(version));
    return version;
}

uint64_t NewTaskId() {
    uint64_t id = gNextTaskId++;
    if (id == 0) {
        id = gNextTaskId++;
    }
    return id;
}

} // namespace

DECL_FUNCTION(bool, downloadTitle,
              const void *tmd,
              size_t tmdSize,
              const void *titleEntry,
              const char *titleVer,
              char *folderName,
              bool inst,
              uint32_t dlDev,
              bool toUSB,
              bool keepFiles,
              void *queueData) {
    (void)titleEntry;
    (void)titleVer;
    (void)folderName;
    (void)toUSB;
    (void)keepFiles;
    (void)queueData;

    const uint64_t titleId = ReadTitleId(tmd);
    const uint16_t titleVersion = ReadTitleVersion(tmd);
    const uint64_t taskId = NewTaskId();

    if (titleId != 0) {
        nusbg::DownloadTask task{};
        task.task_id = taskId;
        task.title_id = titleId;
        task.title_version = titleVersion;
        task.region = 0;
        task.device = static_cast<uint8_t>(dlDev);
        task.install_after_download = inst ? 1 : 0;

        const auto result = nusbg::QueueDownload(task);
        if (result != nusbg::QueueResult::Ok) {
            OSReport("[NUSBG] Failed to queue %016llX: %d\\n",
                     static_cast<unsigned long long>(titleId),
                     static_cast<int>(result));
        } else {
            OSReport("[NUSBG] Queued active download %016llX\\n",
                     static_cast<unsigned long long>(titleId));
        }
    }

    const bool result = real_downloadTitle(
        tmd, tmdSize, titleEntry, titleVer, folderName, inst,
        dlDev, toUSB, keepFiles, queueData);

    // If NUSspli finished the foreground download normally, there is nothing
    // left for Download Management to resume. A task that is interrupted by
    // leaving NUSspli remains queued and is handed off in ON_APPLICATION_ENDS.
    if (result && titleId != 0) {
        nusbg::RemoveDownload(taskId);
        OSReport("[NUSBG] Foreground download completed %016llX\\n",
                 static_cast<unsigned long long>(titleId));
    }

    return result;
}

bool InstallNUSspliHook() {
    const auto status = FunctionPatcher_InitLibrary();
    if (status != FUNCTION_PATCHER_RESULT_SUCCESS) {
        OSReport("[NUSBG] FunctionPatcher init failed: %d\\n", status);
        return false;
    }

    function_replacement_data_t patch =
        REPLACE_FUNCTION_OF_EXECUTABLE_BY_FUNCTION_NAME(
            downloadTitle,
            kTargetTitleIds,
            1,
            "NUSspli.rpx",
            "downloadTitle");

    bool patchedNow = false;
    const auto addStatus =
        FunctionPatcher_AddFunctionPatch(&patch, &gDownloadTitlePatch, &patchedNow);

    if (addStatus != FUNCTION_PATCHER_RESULT_SUCCESS) {
        OSReport("[NUSBG] Failed to register NUSspli downloadTitle hook: %d\\n",
                 addStatus);
        return false;
    }

    OSReport("[NUSBG] NUSspli downloadTitle hook registered (patched=%d)\\n",
             patchedNow ? 1 : 0);
    return true;
}
