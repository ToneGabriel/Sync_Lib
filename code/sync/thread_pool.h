#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <future>
#include <queue>
#include <deque>
#include <vector>
#include <utility>
#include <functional>

#include "_Core.h"


SYNC_BEGIN

/**
 * @brief Enum for job priority in thread_pool internal queue
 * @note Priority might change based on wait time
 */
enum class priority : uint8_t
{
    highest = 0,
    high    = UINT8_MAX / 4,
    normal  = UINT8_MAX / 2,
    low     = UINT8_MAX / 4 * 3,
    lowest  = UINT8_MAX
};  // END priority

DETAIL_BEGIN

/**
 * @brief Helper class to be integrated into a priority queue
 */
class _priority_job
{
private:

    /**
     * @brief User set priority
     */
    priority _prio;

    /**
     * @brief The actual job
     */
    std::function<void(void)> _job;

    /**
     * @brief Insertion time
     */
    typename std::chrono::steady_clock::time_point _timestamp;

public:

    /**
     * @brief Default constructor
     */
    _priority_job() = default;

    /**
     * @brief Constructor that sets priority and job for this object
     * @note Job ownership is transfered
     */
    _priority_job(priority prio, std::function<void(void)>&& job)
        :   _prio(prio),
            _job(std::move(job)),
            _timestamp(std::chrono::steady_clock::now())
    {
        // Empty
    }

    /**
     * @brief Default destructor
     */
    ~_priority_job() = default;

    /**
     * @brief Delete copy constructor and operator
     */
    _priority_job(const _priority_job&)             = delete;
    _priority_job& operator=(const _priority_job&)  = delete;

    /**
     * @brief Move constructor
     * @param other object to be moved
     */
    _priority_job(_priority_job&& other) noexcept
    {
        _move(std::move(other));
    }

    /**
     * @brief Move assign operator
     * @param other object to be moved
     */
    _priority_job& operator=(_priority_job&& other) noexcept
    {
        if (this != &other)
            _move(std::move(other));

        return *this;
    }

public:

    /**
     * @brief Call the stored job if any
     */
    void operator()() const
    {
        if (_job)
            _job();
    }

    /**
     * @brief Number used by `operator<` for comparison in `std::priority_queue`
     * @note Priority might be higher than the original due to wait time
     */
    uint8_t effective_priority() const
    {
        // Subtract from original priority the number of seconds this object waited since timestamp
        auto age    = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - _timestamp).count();
        auto prio   = static_cast<uint8_t>(_prio);

        return (prio <= age) ? 0 : prio - age;
    }

private:

    /**
     * @brief Ownership transfer algorithm
     * @param other object from where to get data
     */
    void _move(_priority_job&& other) noexcept
    {
        _prio       = other._prio;
        _job        = std::move(other._job);
        _timestamp  = std::chrono::steady_clock::now(); // timestamp is refreshed
    }
};  // END _priority_job


/**
 * @brief Operator used in `std::priority_queue` ordering
 * @param left, right object to compare
 * @note Lower numbers mean higher priority
 */
inline bool operator<(const _priority_job& left, const _priority_job& right)
{
    return left.effective_priority() > right.effective_priority();
}

DETAIL_END

/**
 * @brief Class that manages multiple tasks in paralel in an ordered and threadsafe manner
 */
class thread_pool
{
private:
    using _priority_job = detail::_priority_job;

private:

    /**
     * @brief Priority queue for jobs
     */
    std::priority_queue<_priority_job> _pendingJobs;

    /**
     * @brief Vector to store jobs that are not yet in queue
     */
    std::vector<_priority_job> _storedJobs;

    /**
     * @brief Dynamic container for threads. Also join automatically when destroyed
     */
    std::vector<std::jthread> _threads;

    /**
     * @brief Thread safety mutex of this pool
     */
    mutable std::mutex _poolMtx;

    /**
     * @brief Condition Variable for job start notification
     */
    mutable std::condition_variable _poolCV;

    /**
     * @brief Counts jobs done until pool is destroyed
     */
    std::atomic_size_t _jobsDone = 0;

    /**
     * @brief Join flag
     */
    std::atomic_bool _joined = false;

    /**
     * @brief Pause flag
     */
    std::atomic_bool _paused = false;

public:
    // Constructors & Operators

    /**
     * @brief Construct a new thread pool object with default number of threads
     */
    thread_pool()
    {
        restart(std::thread::hardware_concurrency());
    }

    /**
     * @brief Construct a new thread pool object with given number of threads
     * @param nthreads given number of threads
     */
    thread_pool(size_t nthreads)
    {
        restart(nthreads);
    }

    /**
     * @brief Destroy the thread pool object after ALL jobs are done
     */
    ~thread_pool()
    {
        join();
    }

    /**
     * @brief Delete copy/move constructors
     */
    thread_pool(const thread_pool&)             = delete;
    thread_pool(thread_pool&&)                  = delete;

    /**
     * @brief Delete copy/move operators
     */
    thread_pool& operator=(const thread_pool&)  = delete;
    thread_pool& operator=(thread_pool&&)       = delete;

public:
    // Main functions

    /**
     * @brief Get the number of running threads
     */
    size_t thread_count() const
    {
        return _threads.size();
    }

    /**
     * @brief Get the number of executed tasks
     */
    size_t jobs_done() const
    {
        return _jobsDone.load(std::memory_order::relaxed);
    }

    /**
     * @brief Get the number of jobs waiting in queue
     */
    size_t pending_jobs() const
    {
        std::lock_guard lock(_poolMtx);

        return _pendingJobs.size();   
    }

    /**
     * @brief Remove ALL jobs from queue
     */
    void clear_pending_jobs()
    {
        std::lock_guard lock(_poolMtx);

        while (!_pendingJobs.empty())
            _pendingJobs.pop();
    }

    /**
     * @brief Get the number of jobs stored for later execution
     * @note Start them later using `flush_job_storage()`
     */
    size_t stored_jobs() const
    {
        std::lock_guard lock(_poolMtx);

        return _storedJobs.size();
    }

    /**
     * @brief Remove ALL jobs stored for later execution
     */
    void clear_stored_jobs()
    {
        std::lock_guard lock(_poolMtx);

        _storedJobs.clear();
    }

    /**
     * @brief Get the join state of the pool
     */
    bool is_joined() const
    {
        return _joined.load(std::memory_order::relaxed);
    }

    /**
     * @brief Get the pause state of the pool
     */
    bool is_paused() const
    {
        return _paused.load(std::memory_order::relaxed);
    }

    /**
     * @brief Worker threads wait until resume or join
     */
    void pause()
    {
        _paused.store(true, std::memory_order::relaxed);
    }

    /**
     * @brief Resume the execution after pause
     */
    void resume()
    {
        _paused.store(false, std::memory_order::relaxed);
        _poolCV.notify_all();
    }

    /**
     * @brief Close pool and reopen with new number of threads
     * @param nthreads new number of threads
     * @note Pause state is kept
     */
    void restart(size_t nthreads)
    {
        if (is_paused())
        {
            _join();

            _start(nthreads);
        }
        else
        {
            pause();

            _join();

            _start(nthreads);

            resume();
        }
    }

    /**
     * @brief Join threads, but NOT before finishing ALL jobs
     * @note Use `restart()` to create new threads
     */
    void join()
    {
        resume();   // ensure the pool is not paused

        _join();    // threads join after ALL jobs are done
    }

    /**
     * @brief Join threads after finishing running jobs
     * @note If the pool is destroyed after a forced join, the remaining jobs will be lost
     * @note Use `restart()` to create new threads
     */
    void force_join()
    {
        pause();    // wait for running jobs to finish

        _join();    // threads join after running jobs are done
    }

    /**
     * @brief Assign new job with given priority
     * @tparam Functor type of the function object
     * @tparam Args types of the arguments of the function object
     * @param prio priority in queue
     * @param func function object to be called
     * @param args arguments for the call
     * @return A `std::future` to be used later to wait for the function to finish executing and obtain its returned value if it has one
     * @note Use `try`-`catch` block when accessing the return value if the function call throws
     */
    template<class Functor, class... Args>
    std::future<std::invoke_result_t<Functor, Args...>> do_job(priority prio, Functor&& func, Args&&... args)
    {
        using _ReturnType = std::invoke_result_t<Functor, Args...>;

        // Create a packaged_task for the job
        auto safeJobPtr = std::make_shared<std::packaged_task<_ReturnType(void)>>(
                            std::bind(std::forward<Functor>(func), std::forward<Args>(args)...));

        // Move it on the queue (wrapped in a lambda)
        {
            std::lock_guard lock(_poolMtx);
            _pendingJobs.emplace(prio, [safeJobPtr]() { (*safeJobPtr)(); });
        }

        // Unblock a thread
        _poolCV.notify_one();

        return safeJobPtr->get_future();
    }

    /**
     * @brief Assign new job with `priority::normal`
     * @tparam Functor type of the function object
     * @tparam Args types of the arguments of the function object
     * @param func function object to be called
     * @param args arguments for the call
     * @return A `std::future` to be used later to wait for the function to finish executing and obtain its returned value if it has one
     * @note Use `try`-`catch` block when accessing the return value if the function call throws
     */
    template<class Functor, class... Args>
    std::future<std::invoke_result_t<Functor, Args...>> do_job(Functor&& func, Args&&... args)
    {
        return do_job(priority::normal, std::forward<Functor>(func), std::forward<Args>(args)...);
    }

    /**
     * @brief Store new job with given priority. Start later using `flush_job_storage()`
     * @tparam Functor type of the function object
     * @tparam Args types of the arguments of the function object
     * @param prio priority in queue
     * @param func function object to be called
     * @param args arguments for the call
     * @return A `std::future` to be used later to wait for the function to finish executing and obtain its returned value if it has one
     * @note Use `try`-`catch` block when accessing the return value if the function call throws
     */
    template<class Functor, class... Args>
    std::future<std::invoke_result_t<Functor, Args...>> store_job(priority prio, Functor&& func, Args&&... args)
    {
        using _ReturnType = std::invoke_result_t<Functor, Args...>;

        // Create a packaged_task for the job
        auto safeJobPtr = std::make_shared<std::packaged_task<_ReturnType(void)>>(
                            std::bind(std::forward<Functor>(func), std::forward<Args>(args)...));

        // Move it on the vector (wrapped in a lambda) (no lock needed)
        _storedJobs.emplace_back(prio, [safeJobPtr]() { (*safeJobPtr)(); });

        return safeJobPtr->get_future();
    }

    /**
     * @brief Store new job with `priority::normal`. Start later using `flush_job_storage()`
     * @tparam Functor type of the function object
     * @tparam Args types of the arguments of the function object
     * @param func function object to be called
     * @param args arguments for the call
     * @return A `std::future` to be used later to wait for the function to finish executing and obtain its returned value if it has one
     * @note Use `try`-`catch` block when accessing the return value if the function call throws
     */
    template<class Functor, class... Args>
    std::future<std::invoke_result_t<Functor, Args...>> store_job(Functor&& func, Args&&... args)
    {
        return store_job(priority::normal, std::forward<Functor>(func), std::forward<Args>(args)...);
    }

    /**
     * @brief Assign ALL stored jobs to the execution queue
     * @note Does not call `restart()`/`resume()`if `join()`/`pause()` was called previously
     */
    void flush_job_storage()
    {
        {
            std::lock_guard lock(_poolMtx);

            // Move jobs to the execution queue
            while (!_storedJobs.empty())
            {
                _pendingJobs.emplace(std::move(_storedJobs.back()));
                _storedJobs.pop_back();
            }
        }

        // Notify all threads to stop waiting for job
        _poolCV.notify_all();
    }

private:
    // Helpers

    /**
     * @brief Create a number of threads
     * @param nthreads threads to create
     */
    void _start(size_t nthreads)
    {
        _ASSERT(nthreads > 0, "Pool cannot have 0 threads!");

        _joined.store(false, std::memory_order::relaxed);

        while (nthreads--)
            _threads.push_back(std::jthread(std::bind(&thread_pool::_worker_thread, this)));
    }

    /**
     * @brief Signal threads the pool is closed, then join
     */
    void _join()
    {
        // Notify all threads to stop waiting for job
        _joined.store(true, std::memory_order::relaxed);
        _poolCV.notify_all();

        // Destroy thread objects
        // Threads will exit the loop and will join here automatically
        _threads.clear();
    }

    /**
     * @brief Function used by threads. Here is where all the work is done
     */
    void _worker_thread()
    {
        _priority_job job;

        for (;;)
        {
            {   // Empty scope start -> mutex lock and job decision
                std::unique_lock<std::mutex> lock(_poolMtx);

                // Pool is working, but it's paused or there are no jobs -> wait
                // Safeguard against spurious wakeups (while instead of if)
                while (!is_joined() && (is_paused() || _pendingJobs.empty()))
                    _poolCV.wait(lock);

                // Pool is joined
                // Threads finish jobs first if not paused
                if (is_paused() || _pendingJobs.empty())
                    return;

                // Pool is working/joined, not paused and jobs are available
                // Assign job to thread from queue
                job = std::move(const_cast<_priority_job&>(_pendingJobs.top()));
                _pendingJobs.pop();
            }   // Empty scope end -> unlock, can start job

            // Do the job without holding any locks
            job();

            // Count work done (even if throws)
            ++_jobsDone;
        }
    }
};  // END thread_pool

SYNC_END