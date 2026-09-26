#include "nusspli_bg_bridge.h"
#include <stdio.h>
#include <sys/stat.h>

void nusspli_bg_publish(uint64_t task_id,
                        uint64_t title_id,
                        uint16_t title_version,
                        uint8_t device,
                        uint8_t install_after_download) {
    NUSBG_WireTask task = {
        NUSBG_WIRE_MAGIC, NUSBG_WIRE_VERSION, sizeof(NUSBG_WireTask),
        task_id, title_id, title_version, device, install_after_download
    };
    mkdir("sd:/wiiu", 0777);
    mkdir(NUSBG_QUEUE_DIR, 0777);
    char tmp[128], ready[128];
    snprintf(tmp, sizeof(tmp), NUSBG_QUEUE_DIR "/%016llX.tmp",
             (unsigned long long)task_id);
    snprintf(ready, sizeof(ready), NUSBG_QUEUE_DIR "/%016llX.task",
             (unsigned long long)task_id);
    FILE *fp = fopen(tmp, "wb");
    if (!fp) return;
    if (fwrite(&task, sizeof(task), 1, fp) != 1) {
        fclose(fp);
        remove(tmp);
        return;
    }
    fflush(fp);
    fclose(fp);
    rename(tmp, ready);
}
