#include "nus_background_manager.h"
#include "nusspli_bg_bridge.h"
#include <dirent.h>
#include <cstdio>
#include <cstring>
namespace {
constexpr uint32_t kPollEveryYields = 512;
uint32_t gYieldCounter = 0;
bool ReadTaskFile(const char *path, nusbg::DownloadTask &out) {
    FILE *fp = fopen(path, "rb");
    if (!fp) return false;
    NUSBG_WireTask wire{};
    const bool ok = fread(&wire, sizeof(wire), 1, fp) == 1;
    fclose(fp);
    if (!ok || wire.magic != NUSBG_WIRE_MAGIC || wire.version != NUSBG_WIRE_VERSION ||
        wire.size != sizeof(NUSBG_WireTask) || wire.task_id == 0 || wire.title_id == 0) return false;
    out = {};
    out.task_id = wire.task_id;
    out.title_id = wire.title_id;
    out.title_version = wire.title_version;
    out.device = wire.device;
    out.install_after_download = wire.install_after_download;
    snprintf(out.tmd_url, sizeof(out.tmd_url), "nusspli:%016llX",
             (unsigned long long) out.title_id);
    return true;
}
void PollNUSspliQueue() {
    if (++gYieldCounter < kPollEveryYields) return;
    gYieldCounter = 0;
    DIR *dir = opendir(NUSBG_QUEUE_DIR);
    if (!dir) return;
    while (dirent *entry = readdir(dir)) {
        if (!entry->d_name) continue;
        const char *suffix = strrchr(entry->d_name, '.');
        if (!suffix || strcmp(suffix, ".task") != 0) continue;
        char path[256];
        snprintf(path, sizeof(path), NUSBG_QUEUE_DIR "/%s", entry->d_name);
        nusbg::DownloadTask task{};
        if (ReadTaskFile(path, task) && nusbg::QueueDownload(task) == nusbg::QueueResult::Ok) remove(path);
    }
    closedir(dir);
}
}
extern "C" void NUSBG_PollFromNUSspli() { PollNUSspliQueue(); }
