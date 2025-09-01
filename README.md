# Sync

**Version 1.1.0-beta**

**Sync** is a lightweight, header-only C++ library providing concurrency utilities with a design inspired by `boost::asio` library.   
It offers a simple yet efficient foundation for multithreading through the main components:

- **thread_pool**: A scalable pool for executing tasks in parallel.
- **task_context**: A lightweight wrapper over the shared scheduler for task execution.
- **multilogger**: A thread-safe logger that writes to multiple output streams at once.

---

## Contents

<details>
<summary><b>Highlights</b></summary>

- Header-only – No compilation required; just include the headers.
- Modern C++20 Design – Leverages lambdas, smart pointers and RAII. 
- Unified Scheduler – Both `thread_pool` and `task_context` share the same scheduler implementation for efficient task management.
- Simple Interface – Submit tasks via `sync::post()` and let the executor handle them.
- Priority-Based Scheduling – Scheduler uses a priority queue; tasks can be posted with custom priority levels.
- Safe Execution – `sync::post()` returns `std::future<T>` so results or exceptions can be retrieved.
- Safe Logs – `sync::multilogger` enables simultaneous logging to multiple output streams (including custom ones).
- Well-tested – The project includes unit tests and builds the corresponding test executables.

</details>
<!-- END Highlights -->

<details>
<summary><b>Usage</b></summary>

```C++
#include "sync/thread_pool.hpp"

void simple_task()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

int main()
{
    sync::thread_pool pool(5);

    // assign tasks to thread_pool and create std::futures for results
    auto res1 = sync::post(pool, sync::priority::high, simple_task);
    auto res2 = sync::post(pool, simple_task);  // medium priority by default
    // ... any number of tasks

    // futures can block to wait for results
    res1.get();
    res2.get();

    // close the pool
    pool.join();

    return 0;
}
```

</details>
<!-- END Usage -->

<details>
<summary><b>Headers</b></summary>

- `task_context.hpp`
- `thread_pool.hpp`

</details>
<!-- END Headers -->

---

## Requirements

- **C++ Compiler**: C++20 compliant
- **Build System**: CMake (≥ 3.22.1), Ninja  
- **Testing Frameworks**:  
  - [GoogleTest](https://github.com/google/googletest) (auto fetched via CMake)  

---

## Installation & Build

```bash
# Clone the repository
git clone https://github.com/ToneGabriel/Sync_Lib.git
cd Sync_Lib

# Create a build directory
cmake -G "Ninja" -B build

# Build the project
cmake --build build
```

Or simply run the script `RUN_TESTS` and the build is done automatically.   
The results can be found in `logs` folder.
