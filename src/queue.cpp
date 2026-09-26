#include "nus_background_manager.h"

#include <mutex>
#include <vector>

namespace {
std::mutex gQueueMutex;
std::vector<nusbg::DownloadTask> gQueue;

bool IsValid(const nusbg::DownloadTask& task) {
    return task.task_id != 0 && task.title_id != 0 && task.tmd_url[0] != '\0';
}
}

namespace nusbg {

QueueResult QueueDownload(const DownloadTask& task) {
    if (!IsValid(task)) {
        return QueueResult::Invalid;
    }

    std::lock_guard<std::mutex> lock(gQueueMutex);

    if (gQueue.size() >= kMaxQueuedDownloads) {
        return QueueResult::QueueFull;
    }

    gQueue.push_back(task);
    return QueueResult::Ok;
}

void ClearQueue() {
    std::lock_guard<std::mutex> lock(gQueueMutex);
    gQueue.clear();
}

std::size_t GetQueueSize() {
    std::lock_guard<std::mutex> lock(gQueueMutex);
    return gQueue.size();
}

std::vector<DownloadTask> SnapshotQueue() {
    std::lock_guard<std::mutex> lock(gQueueMutex);
    return gQueue;
}

void ReplaceQueue(const std::vector<DownloadTask>& tasks) {
    std::lock_guard<std::mutex> lock(gQueueMutex);
    gQueue = tasks;
}

} // namespace nusbg
