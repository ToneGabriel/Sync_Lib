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


/**
 * @brief Used for thread_pool task priority management
 * @note Lower numbers mean higher priority
 */
using priority_t = uint8_t;


/**
 * @brief Namespace for pre-defined priorities
 */
namespace priority
{
    constexpr priority_t highest    = 0;
    constexpr priority_t high       = UINT8_MAX / 4;
    constexpr priority_t normal     = UINT8_MAX / 2;
    constexpr priority_t low        = UINT8_MAX / 4 * 3;
    constexpr priority_t lowest     = UINT8_MAX;
}   // END priority


/**
 * @brief Class that manages multiple tasks in paralel in an ordered and threadsafe manner
 */
class thread_pool
{
private:

    /**
     * @brief Helper class to be integrated into a priority queue
     */
    class _priority_job
    {
    private:

        /**
         * @brief Used to order the priority queue
         */
        priority_t _prio;

        /**
         * @brief The actual job
         */
        std::function<void(void)> _job;

        /**
         * @brief Tie-breaker for operator<
         */
        std::chrono::steady_clock::time_point _timestamp;

    public:

        /**
         * @brief Default constructor
         */
        _priority_job() = default;

        /**
         * @brief Constructor that sets priority and job for this object
         * @note Job ownership is transfered
         */
        _priority_job(priority_t prio, std::function<void(void)>&& job)
            : _prio(prio), _job(std::move(job)), _timestamp(std::chrono::steady_clock::now()) { /* Empty */ }

        /**
         * @brief Default destructor
         */
        ~_priority_job() = default;

    public:

        /**
         * @brief Change priority
         * @param newPrio The new priority
         */
        void set_priority(priority_t newPrio)
        {
            _prio = newPrio;
        }

        /**
         * @brief Call the stored job if any
         */
        void operator()() const
        {
            if (_job)
                _job();
        }

    public:

        /**
         * @brief Operator used by std::less in std::priority_queue ordering
         * @note Time of submission is used if priorities are equal. Maintains insertion order
         */
        friend bool operator<(const _priority_job& left, const _priority_job& right)
        {
            return (left._prio == right._prio)  ? left._timestamp < right._timestamp
                                                : left._prio < right._prio;
        }
    };  // END _priority_job

private:

    /**
     * @brief Priority queue for jobs
     */
    std::priority_queue<_priority_job, std::deque<_priority_job>> _jobs;

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
     * @brief Shutdown flag
     */
    std::atomic_bool _shutdown = false;

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
    thread_pool(const size_t nthreads)
    {
        restart(nthreads);
    }

    /**
     * @brief Destroy the thread pool object after ALL jobs are done
     */
    ~thread_pool()
    {
        shutdown();
    }

    thread_pool(const thread_pool&)             = delete;
    thread_pool(thread_pool&&)                  = delete;

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

        return _jobs.size();   
    }

    /**
     * @brief Remove ALL jobs from queue
     */
    void clear_pending_jobs()
    {
        std::lock_guard lock(_poolMtx);

        while (!_jobs.empty())
            _jobs.pop();
    }

    /**
     * @brief Get the shutdown state of the pool
     */
    bool is_shutdown() const
    {
        return _shutdown.load(std::memory_order::relaxed);
    }

    /**
     * @brief Get the pause state of the pool
     */
    bool is_paused() const
    {
        return _paused.load(std::memory_order::relaxed);
    }

    /**
     * @brief Worker threads wait until resume or shutdown
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
     * @brief Pause, close pool and reopen with new number of threads, resume
     * @param nthreads new number of threads
     */
    void restart(const size_t nthreads)
    {
        pause();

        _close_pool();  // threads join after running jobs are done

        _open_pool(nthreads);   // new threads are created and started

        resume();
    }

    /**
     * @brief Destroy threads, but NOT before finishing ALL jobs
     */
    void shutdown()
    {
        resume();   // ensure the pool is not paused

        _close_pool();  // threads join after ALL jobs are done
    }

    /**
     * @brief Destroy threads after finishing running jobs
     * @note If the pool is destroyed after a forced shutdown, the remaining jobs will be lost
     */
    void force_shutdown()
    {
        pause();

        _close_pool();  // threads join after running jobs are done
    }

    /**
     * @brief Assign new job with priority
     * @tparam Functor type of the function object
     * @tparam Args types of the arguments of the function object
     * @param prio priority in queue
     * @param func function object to be called
     * @param args arguments for the call
     * @return A future to be used later to wait for the function to finish executing and obtain its returned value if it has one
     * @note Use try-catch block when accessing the return value as it may throw
     */
    template<class Functor, class... Args>
    std::future<std::invoke_result_t<Functor, Args...>> do_job(priority_t prio, Functor&& func, Args&&... args)
    {
        using _ReturnType = std::invoke_result_t<Functor, Args...>;

        // Create a packaged_task for the job
        auto safeJobPtr = std::make_shared<std::packaged_task<_ReturnType(void)>>(
                            std::bind(std::forward<Functor>(func), std::forward<Args>(args)...));

        // Move it on the queue (wrapped in a lambda)
        {
            std::lock_guard lock(_poolMtx);
            _jobs.emplace(prio, [safeJobPtr]() { (*safeJobPtr)(); });
        }

        // Unblock a thread
        _poolCV.notify_one();

        return safeJobPtr->get_future();
    }

private:
    // Helpers

    /**
     * @brief Create a number of threads
     * @param nthreads threads to create
     */
    void _open_pool(const size_t nthreads)
    {
        _ASSERT(nthreads > 0, "Pool cannot have 0 threads!");

        _shutdown.store(false, std::memory_order::relaxed);

        for (size_t i = 0; i < nthreads; ++i)
        {
            std::jthread t = std::jthread(std::bind(&thread_pool::_worker_thread, this));
            _threads.push_back(std::move(t));
        }
    }

    /**
     * @brief Signal threads the pool is closed, then join
     */
    void _close_pool()
    {
        // Notify all threads to stop waiting for job
        _shutdown.store(true, std::memory_order::relaxed);
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
                while (!is_shutdown() && (is_paused() || _jobs.empty()))
                    _poolCV.wait(lock);

                // Pool is closed
                // Threads finish jobs first if not paused
                if (is_paused() || _jobs.empty())
                    return;

                // Pool is working/closed, not paused and jobs are available
                // Assign job to thread from queue
                job = std::move(const_cast<_priority_job&>(_jobs.top()));
                _jobs.pop();
            }   // Empty scope end -> unlock, can start job

            // Do the job without holding any locks
            job();

            // Count work done (even if throws)
            ++_jobsDone;
        }
    }
};  // END thread_pool
