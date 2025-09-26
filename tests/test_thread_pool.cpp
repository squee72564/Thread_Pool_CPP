#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <future>
#include <stdexcept>
#include "thread_pool.hpp"

int main() {
    using namespace std::chrono_literals;

    std::cout << "Creating thread pool with 4 threads...\n";
    ThreadPool pool(4);

    // ----------------------
    // Test 1: simple void tasks
    // ----------------------
    std::cout << "Test 1: Simple void tasks\n";
    std::atomic<int> counter = 0;
    std::vector<std::future<void>> f1;

    for (int i = 0; i < 10; ++i) {
        f1.push_back(pool.enqueue([&counter, i]() {
            counter += 1;
            std::cout << "Task " << i << " executed by thread " 
                      << std::this_thread::get_id() << "\n";
        }));
    }

    for (auto &future : f1) {
        future.get();
    }
    std::cout << "Counter value: " << counter << " (expected 10)\n\n";

    // ----------------------
    // Test 2: tasks with return values
    // ----------------------
    std::cout << "Test 2: Tasks with return values\n";
    std::vector<std::future<int>> futures;

    for (int i = 0; i < 8; ++i) {
        futures.push_back(pool.enqueue([i]() {
            std::this_thread::sleep_for(20ms);
            return i * i;
        }));
    }

    for (size_t i = 0; i < futures.size(); ++i) {
        int result = futures[i].get();
        std::cout << "Task " << i << " returned " << result << "\n";
    }

    // ----------------------
    // Test 3: tasks that throw exceptions
    // ----------------------
    std::cout << "\nTest 3: Tasks that throw exceptions\n";
    auto fut = pool.enqueue([]() -> int {
        throw std::runtime_error("Task failure!");
        return 42;
    });

    try {
        int result = fut.get();
        (void)result;
    } catch (const std::exception &e) {
        std::cout << "Caught exception from task: " << e.what() << "\n";
    }

    // ----------------------
    // Test 4: many tasks, stress test
    // ----------------------
    std::cout << "\nTest 4: Stress test with 10000 tasks\n";
    std::atomic<int> stress_counter = 0;
    std::vector<std::future<void>> stress_futures;

    for (int i = 0; i < 10000; ++i) {
        stress_futures.push_back(pool.enqueue([&stress_counter, i]() {
            if (i % 10 == 0) std::this_thread::sleep_for(5ms);
            stress_counter.fetch_add(1, std::memory_order_relaxed);
        }));
    }

    for (auto &f : stress_futures) f.get();
    std::cout << "Stress counter: " << stress_counter << " (expected 10000)\n";

    // ----------------------
    // Test 5: enqueue after pool is stopped
    // ----------------------
    std::cout << "\nTest 5: Enqueue after pool stopped\n";
    pool.stop();

    try {
        pool.enqueue([]() { std::cout << "This should not run!\n"; });
    } catch (const std::runtime_error &e) {
        std::cout << "Correctly caught exception: " << e.what() << "\n";
    }


    // ----------------------
    // Test 6: restart queue and enqueue
    // ----------------------
    std::cout << "\nTest 6: Enqueue after restarting pool\n";
    pool.start();
    std::future<void> f2;
    try {
        f2 = pool.enqueue([]() { std::cout << "This should run!\n"; });
    } catch (const std::runtime_error &e) {
        std::cout << "Inorrectly caught exception: " << e.what() << "\n";
    }

    f2.get();

    std::cout << "\nAll tests completed.\n";
    return 0;
}
