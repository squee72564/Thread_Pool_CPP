This is a simple C++ thread pool implementation that allows you to execute tasks concurrently. It supports arbitrary functions with arbitrary return types and provides futures for retrieving results.

# Building
You can build it easily with CMake

```bash
git clone https://github.com/squee72564/Thread_Pool_CPP.git
cd Thread_Pool_CPP
mkdir build
cmake -S . -B build
cmake --build build
```

Or alternatively if you want to build tests as well use

```
cmake -S . -B build -DBUILD_TESTS=ON
cmake --build build
```

# Usage
```cpp
ThreadPool pool(4);

auto square_root_i = [](int x){ return x * x; };

// Enqueue a task
auto fut = pool.enqueue(
    square_root_i, 5
);

// Get result from the task
int result = fut.get();

// Stop the pool
pool.stop();

try {
    // Throws as pool is stopped
    auto fut2 = pool.enqueue(
        square_root_i, 10
    );

    int result2 = fut2.get();

} catch (const std::runtime_error& e) {
    // Do something
}

// Start again with 8 threads in pool
pool.start(8);

// Functions that throw errors can also be caught
auto fn_should_throw = [](){ throw std::runtime_error{"This throws."}; };

auto fut3 = pool.enqueue(fn_should_throw);

try {
    fut3.get();
} catch (const std::runtime_error& e) {
    // Do something with caught error
}

```
