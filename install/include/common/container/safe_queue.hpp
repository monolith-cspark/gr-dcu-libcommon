
#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>

namespace GR {
namespace LIBCOMMON {
template <typename T>
class SafeQueue {
public:
    void push(T item) {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(std::move(item));
        cond_.notify_one();
    }

    T pop() {
        std::unique_lock<std::mutex> lock(mutex_);
        cond_.wait(lock, [this] { return !queue_.empty() || !running_; });
        if (queue_.empty()) return {};
        T item = std::move(queue_.front());
        queue_.pop();
        return item;
    }

    void stop() {
        { std::lock_guard<std::mutex> lock(mutex_); running_ = false; }
        cond_.notify_all();
    }

private:
    std::queue<T> queue_;
    std::mutex mutex_;
    std::condition_variable cond_;
    bool running_ = true;
};


} // namespace LIBCOMMON
} // namespace GR