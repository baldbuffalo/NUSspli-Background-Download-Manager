#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
#define NUSBG_WIRE_MAGIC 0x4E42474DU
#define NUSBG_WIRE_VERSION 1
#define NUSBG_QUEUE_DIR "sd:/wiiu/nusspli-background"
typedef struct NUSBG_WireTask {
    uint32_t magic;
    uint16_t version;
    uint16_t size;
    uint64_t task_id;
    uint64_t title_id;
    uint16_t title_version;
    uint8_t device;
    uint8_t install_after_download;
} NUSBG_WireTask;
#ifdef __cplusplus
}
#endif
