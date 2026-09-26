#include "nus_background_manager.h"

#include <mutex>
#include <vector>

namespace {
std::mutex gQueueMutex;
std::vector<nusbg::DownloadTask> gQueue;

bool IsValid(const nusbg::DownloadTask& task) {
    return task.task_id != 0 && task.title_id != 0;
}
}

namespace nusbg {

QueueResult QueueDownload(const DownloadTask& task) {
    if (!IsValid(task)) {
        return QueueResult::Invalid;
    }

    std::lock_guard<std::mutex> lock(gQueueMutex);

    for (const auto& queued : gQueue) {
        if (queued.task_id == task.task_id) {
            return QueueResult::Ok;
        }
    }

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

bool RemoveDownload(uint64_t task_id) {
    std::lock_guard<std::mutex> lock(gQueueMutex);
    for (auto it = gQueue.begin(); it != gQueue.end(); ++it) {
        if (it->task_id == task_id) {
            gQueue.erase(it);
            return true;
        }
    }
    return false;
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
