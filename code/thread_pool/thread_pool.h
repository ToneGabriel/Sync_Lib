#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <future>
#include <map>
#include <vector>
#include <utility>
#include <functional>

#include "_Core.h"


/**
 * @brief Enum for thread_pool task priority management
 * @see thread_pool
 * @see thread_pool::do_job()
 */
enum class job_priority
{
    highest,
    high,
    normal,
    low,
    lowest
};  // END job_priority


/**
 * @brief Class that manages multiple tasks in an ordered and threadsafe manner
 */
class thread_pool
{
private:
    std::multimap<  job_priority,
                    std::function<void(void)>>  _priorityJobQueue;      // Priority queue for jobs
    std::vector<std::thread>                    _workerThreads;         // Dynamic container for threads
    mutable std::mutex                          _poolMtx;               // Thread safety mutex of this pool
    mutable std::condition_variable             _poolCV;                // CV for job start notification

    std::atomic_size_t                          _jobsDone   = 0;        // Counts jobs done until destroyed
    bool                                        _shutdown   = false;    // Shutdown flag
    bool                                        _paused     = false;    // Pause flag

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
        return _workerThreads.size();
    }

    /**
     * @brief Get the number of executed tasks
     */
    size_t jobs_done() const
    {
        return _jobsDone;
    }

    /**
     * @brief Get the number of jobs waiting in queue
     */
    size_t pending_jobs() const
    {
        std::lock_guard lock(_poolMtx);

        return _priorityJobQueue.size();   
    }

    /**
     * @brief Remove ALL jobs from queue
     */
    void clear_pending_jobs()
    {
        std::lock_guard lock(_poolMtx);

        _priorityJobQueue.clear();
    }

    /**
     * @brief Get the pause state of the pool
     */
    bool is_paused() const
    {
        std::lock_guard lock(_poolMtx);

        return _paused;
    }

    /**
     * @brief Worker threads wait until resume or shutdown
     */
    void pause()
    {
        std::lock_guard lock(_poolMtx);

        _paused = true;
    }

    /**
     * @brief Resume the execution after pause
     */
    void resume()
    {
        std::lock_guard lock(_poolMtx);

        _paused = false;
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
     */
    template<class Functor, class... Args>
    std::future<std::invoke_result_t<Functor, Args...>> do_job( job_priority prio,
                                                                Functor&& func,
                                                                Args&&... args)
    {
        using ReturnType = std::invoke_result_t<Functor, Args...>;

        // Create a packaged_task for the job
        auto safeJobPtr = std::make_shared<std::packaged_task<ReturnType()>>(
        std::bind(std::forward<Functor>(func), std::forward<Args>(args)...));

        // Move it on the queue (wrapped in a lambda) and unblock a thread
        {
            std::lock_guard lock(_poolMtx);

            _priorityJobQueue.emplace(prio, [safeJobPtr]() { (*safeJobPtr)(); });
            _poolCV.notify_one();
        }

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

        _shutdown = false;

        for (size_t i = 0; i < nthreads; ++i)
        {
            std::thread t = std::thread(std::bind(&thread_pool::_worker_thread, this));
            _workerThreads.push_back(std::move(t));
        }
    }

    /**
     * @brief Signal threads the pool is closed, then join
     */
    void _close_pool()
    {
        // Notify all threads to stop waiting for job
        {
            std::lock_guard lock(_poolMtx);

            _shutdown = true;
            _poolCV.notify_all();
        }

        // Threads will exit the loop and will join here
        for (auto& t : _workerThreads)
            t.join();

        // Destroy thread objects
        _workerThreads.clear();
    }

    /**
     * @brief Function used by threads. Here is where all the work is done
     */
    void _worker_thread()
    {
        std::function<void(void)> job;

        for (;;)
        {
            {   // Empty scope start -> mutex lock and job decision
                std::unique_lock<std::mutex> lock(_poolMtx);

                // Pool is working, but it's paused or there are no jobs -> wait
                // Safeguard against spurious wakeups (while instead of if)
                while (!_shutdown && (_paused || _priorityJobQueue.empty()))
                    _poolCV.wait(lock);

                // Pool is closed
                // Threads finish jobs first if not paused
                if (_paused || _priorityJobQueue.empty())
                    return;

                // Pool is working/closed, not paused and jobs are available
                // Assign job to thread from queue
                auto it = _priorityJobQueue.begin();
                job = std::move(it->second);
                _priorityJobQueue.erase(it);
            }   // Empty scope end -> unlock, can start job

            // Do the job without holding any locks
            job();

            // Count work done (even if throws)
            ++_jobsDone;
        }
    }
};  // END thread_pool
