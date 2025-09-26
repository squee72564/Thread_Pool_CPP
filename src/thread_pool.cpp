#include "thread_pool.hpp"

ThreadPool::ThreadPool(size_t numThreads)
    : workers{}
    , tasks{}
    , queueMutex{}
    , condition{}
    , stopFlag{false}
    , num_threads{numThreads}
{
    workers.reserve(numThreads);
    start(numThreads);
}

ThreadPool::~ThreadPool() {
    stop();
}

void ThreadPool::stop() {
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        if (stopFlag) return;
        stopFlag = true;
    }

    condition.notify_all();
    for (std::thread &worker : workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }

    workers.clear();
}

void ThreadPool::start(size_t numThreads) {
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        if (!workers.empty()) return;
        stopFlag = false;
        num_threads = numThreads;
    }

    // spawn new threads
    for (size_t i = 0; i < num_threads; ++i) {
        workers.emplace_back([this] {
            while (true) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(queueMutex);
                    condition.wait(lock, [this] { return stopFlag || !tasks.empty(); });
                    if (stopFlag && tasks.empty()) return;
                    task = std::move(tasks.front());
                    tasks.pop();
                }
                task();
            }
        });
    }
}
