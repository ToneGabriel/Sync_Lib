#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <map>
#include <vector>
#include <utility>
#include <functional>

#include "_Core.h"


enum class job_priority
{
    highest,
    high,
    normal,
    low,
    lowest
};  // END job_priority


class thread_pool
{
private:
    std::multimap<  job_priority,
                    std::function<void(void)>>  _priorityJobQueue;

    std::vector<std::thread>                    _workerThreads;         // Managed worker threads
    std::mutex                                  _poolMtx;               // Mutex for job assign and control
    std::condition_variable                     _poolCV;                // CV for job assign and control

    std::atomic_size_t                          _jobsDone   = 0;        // Counts jobs done until destroyed
    bool                                        _shutdown   = false;    // Control shutdown
    bool                                        _paused     = false;    // Control pause

    std::vector<std::exception_ptr>             _exceptions;            // Buffer for exceptions
    std::mutex                                  _exceptionsMtx;         // Mutex for exception buffer

public:
    // Constructors & Operators

    // Open pool with default number of threads
    thread_pool()
    {
        (void)restart(std::thread::hardware_concurrency());
    }

    // Open pool with given number of threads
    thread_pool(const size_t nthreads)
    {
        (void)restart(nthreads);
    }

    // Destroy pool after ALL jobs are done
    ~thread_pool()
    {
        (void)shutdown();
    }

    thread_pool(const thread_pool&)             = delete;
    thread_pool& operator=(const thread_pool&)  = delete;

public:
    // Main functions

    // Get current thread number
    size_t thread_count() const
    {
        return _workerThreads.size();
    }

    // Get jobs finished
    size_t jobs_done() const
    {
        return _jobsDone;
    }

    void clear_pending_jobs()
    {
        std::lock_guard lock(_poolMtx);

        _priorityJobQueue.clear();
    }

    // Worker threads wait until resume
    void pause()
    {
        std::lock_guard lock(_poolMtx);

        _paused = true;
    }

    // Resume the execution
    // Worker threads are notified and can now get jobs
    void resume()
    {
        std::lock_guard lock(_poolMtx);

        _paused = false;
        _poolCV.notify_all();
    }

    // Pause, refresh the thread vector with new thread count, resume
    // Return caught exceptions
    std::vector<std::exception_ptr> restart(const size_t nthreads)
    {
        std::vector<std::exception_ptr> ret;

        pause();

        _close_pool();  // threads join after running jobs are done

        std::swap(_exceptions, ret);

        _open_pool(nthreads);   // new threads are created and started

        resume();

        return ret;
    }

    // Destroy threads, but NOT before finishing ALL jobs
    // Return caught exceptions
    std::vector<std::exception_ptr> shutdown()
    {
        std::vector<std::exception_ptr> ret;

        resume();   // ensure the pool is not paused

        _close_pool();  // threads join after ALL jobs are done

        std::swap(_exceptions, ret);

        return ret;
    }

    // Destroy threads after finishing running jobs
    // Return caught exceptions
    // NOTE: If the pool is destroyed after a forced shutdown, the remaining jobs will be lost
    std::vector<std::exception_ptr> force_shutdown()
    {
        std::vector<std::exception_ptr> ret;

        pause();

        _close_pool();  // threads join after running jobs are done

        std::swap(_exceptions, ret);

        return ret;
    }

    // Assign new job with priority
    void do_job(std::function<void(void)>&& newJob, job_priority prio = job_priority::normal)
    {
        // Move a job on the queue and unblock a thread
        std::lock_guard lock(_poolMtx);

        // No longer accepting jobs
        if (_shutdown)
            return;

        // Can accept jobs (even if paused)
        _priorityJobQueue.emplace(prio, std::move(newJob));
        _poolCV.notify_one();
    }

private:
    // Helpers

    // Create a number of threads
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

    // Signal threads the pool is closed, then join
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

    // Worker
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
            // Store exceptions
            try
            {
                job();
            }
            catch (const std::exception&)   // Standard handle
            {
                std::lock_guard lock(_exceptionsMtx);
                _exceptions.push_back(std::current_exception());
            }
            catch (...) // Convert unknown exception to a standard exception
            {
                std::lock_guard lock(_exceptionsMtx);
                std::runtime_error e("Job threw an unknown exception");
                _exceptions.push_back(std::make_exception_ptr(e));
            }

            // Count work done (even if throws)
            ++_jobsDone;
        }
    }
};  // END thread_pool
