#include <ratio>
#include <vector>
#include <iostream>
#include <random>
#include <chrono>
#include <future>
#include <thread>

#include "thread_pool.hpp"

double heavy_compute(double x) {
    double result{0.0};

    for (int i = 0; i < 200; ++i) {
        result += std::sin(x) * std::cos(x) + std::sqrt(std::abs(x));
        x += 0.001;
    }

    return result;
}

double sequential_sum(const std::vector<double>& v) {
    double sum{0};
    for (const double& x : v) sum += heavy_compute(x);
    return sum;
}

double naive_parallel_sum(const std::vector<double>& v, int num_threads) {
    std::vector<std::thread> threads;
    threads.reserve(num_threads);

    std::vector<double> partial_sums(num_threads, 0);
    size_t chunk_size = v.size() / num_threads;

    for (int t = 0; t < num_threads; ++t) {
        size_t start = t * chunk_size;
        size_t end = (t == num_threads - 1) ? v.size() : start + chunk_size;
        threads.emplace_back([&v, &partial_sums, t, start, end]() {
            for (size_t i = start; i < end; ++i)
                partial_sums[t] += heavy_compute(v[i]);
        });
    }

    for (auto& th : threads) th.join();

    double sum = 0;
    for (double s : partial_sums) sum += s;
    return sum;
}

double threadpool_sum(ThreadPool& pool, const std::vector<double>& v, int num_threads) {
    std::vector<std::future<double>> futures;
    futures.reserve(num_threads);

    size_t chunk_size = v.size() / num_threads;

    for (int t = 0; t < num_threads; ++t) {
        size_t start = t * chunk_size;
        size_t end = (t == num_threads-1) ? v.size() : start + chunk_size;

        futures.emplace_back(
            pool.enqueue(
                [start, end, &v]() {
                    double partial = 0;
                    for (size_t i = start; i < end; ++i) {
                        partial += heavy_compute(v[i]);
                    }

                    return partial;
                }
            )
        );
    }

    int sum = 0;

    for (auto& f : futures) {
        sum += f.get();
    }

    return sum;
}

double threadpool_sum_dynamic(ThreadPool& pool, const std::vector<double>& v, size_t chunk_size = 64000) {
    std::vector<std::future<double>> futures;
    size_t num_chunks = (v.size() + chunk_size - 1) / chunk_size;

    for (size_t i = 0; i < num_chunks; ++i) {
        size_t start = i * chunk_size;
        size_t end = std::min(start + chunk_size, v.size());

        futures.push_back(pool.enqueue([start,end,&v]() -> double {
            double partial = 0;
            for (size_t j = start; j < end; ++j)
                partial += heavy_compute(v[j]);
            return partial;
        }));
    }

    double sum = 0;
    for (auto &f : futures)
        sum += f.get();

    return sum;
}

int main() {
    const size_t N = 10'000'000;
    const int num_threads = std::thread::hardware_concurrency();

    // Random vector initialization in [-2,2]
    std::vector<double> v;
    v.reserve(N);
    std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<double> dist(-2.0, 2.0);

    for (size_t i = 0; i < N; ++i) v.emplace_back(dist(rng));

    // Sequential
    auto t1 = std::chrono::high_resolution_clock::now();
    double seq_sum = sequential_sum(v);
    auto t2 = std::chrono::high_resolution_clock::now();
    std::cout << "Sequential sum: " << seq_sum
              << " in "
              << std::chrono::duration<double, std::milli>(t2-t1).count()
              << " ms\n";

    std::cout << "Using " << num_threads << " threads.\n";

    // Naive threads
    t1 = std::chrono::high_resolution_clock::now();
    double naive_sum = naive_parallel_sum(v, num_threads);
    t2 = std::chrono::high_resolution_clock::now();
    std::cout << "Naive threads sum: " << naive_sum
              << " in "
              << std::chrono::duration<double, std::milli>(t2-t1).count()
              << " ms\n";

    {
        ThreadPool pool(num_threads);

        // Thread pool
        t1 = std::chrono::high_resolution_clock::now();
        double pool_sum = threadpool_sum(pool, v, num_threads);
        t2 = std::chrono::high_resolution_clock::now();
        std::cout << "Thread pool sum: " << pool_sum
                  << " in "
                  << std::chrono::duration<double, std::milli>(t2-t1).count()
                  << " ms\n";

    }

    {
        ThreadPool pool(num_threads);
        
        // Thread pool dynamic chunking
        t1 = std::chrono::high_resolution_clock::now();
        double pool_dyn_sum = threadpool_sum_dynamic(pool, v);
        t2 = std::chrono::high_resolution_clock::now();
        std::cout << "Thread pool dynamic sum: " << pool_dyn_sum
                  << " in "
                  << std::chrono::duration<double, std::milli>(t2-t1).count()
                  << " ms\n";
    }

    return 0;
}
