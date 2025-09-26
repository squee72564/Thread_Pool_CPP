#include <vector>
#include <thread>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <future>
#include <atomic>

class ThreadPool {
public:
    ThreadPool(size_t numThreads = std::thread::hardware_concurrency());
    ~ThreadPool();

    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args) 
        -> std::future<typename std::invoke_result<F, Args...>::type>
    {
        using return_type = typename std::invoke_result<F, Args...>::type;

        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );

        std::future<return_type> res = task->get_future();
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            if (stopFlag)
                throw std::runtime_error("enqueue on stopped ThreadPool");
            tasks.emplace(
                [task]() { (*task)(); }
            );
        }
        condition.notify_one();
        return res;
    }

    void stop();
    void start(size_t num_threads);

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;

    std::mutex queueMutex;
    std::condition_variable condition;
    std::atomic<bool> stopFlag;
    size_t num_threads;
};
