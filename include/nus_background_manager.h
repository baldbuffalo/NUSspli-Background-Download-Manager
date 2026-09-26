#pragma once

#include <cstddef>
#include <cstdint>

namespace nusbg {

constexpr uint32_t kAbiVersion = 1;
constexpr std::size_t kMaxQueuedDownloads = 32;
constexpr std::size_t kMaxStringLength = 255;

struct DownloadTask {
    uint64_t task_id;
    uint64_t title_id;
    uint16_t title_version;
    uint16_t region;
    uint8_t device;
    uint8_t install_after_download;
    uint8_t reserved[2];

    char tmd_url[kMaxStringLength + 1];
    char ticket_url[kMaxStringLength + 1];
};

enum class QueueResult : int32_t {
    Ok = 0,
    QueueFull = -1,
    Invalid = -2,
};

QueueResult QueueDownload(const DownloadTask& task);
void ClearQueue();
std::size_t GetQueueSize();

void HandoffQueuedDownloads();

} // namespace nusbg
