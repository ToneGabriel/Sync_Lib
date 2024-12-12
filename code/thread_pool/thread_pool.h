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


template<class Container, class = void>
struct _HasValidMethods : std::false_type {};

template<class Container>
struct _HasValidMethods<Container, std::void_t< decltype(std::declval<Container>().size()),
                                                decltype(std::declval<Container>().begin()),
                                                decltype(std::declval<Container>().end())>> : std::true_type {};

template<class Container>
using _EnableJobsFromContainer_t =
            std::enable_if_t<std::conjunction_v<
                                    std::is_same<typename Container::value_type, std::function<void(void)>>,
                                    std::is_convertible<typename std::iterator_traits<typename Container::iterator>::iterator_category, std::forward_iterator_tag>,
                                    _HasValidMethods<Container>>,
            bool>;


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
    std::multimap<job_priority, std::function<void(void)>>  _validJobQueue;     // Queue from where the jobs are assigned to workers
    std::multimap<job_priority, std::function<void(void)>>  _idleJobQueue;      // Queue used when paused

    std::vector<std::thread>            _workerThreads;                         // Actual workers
    std::mutex                          _poolMtx;                               // Mutex for job assign and control
    std::condition_variable             _poolCV;                                // CV for job assign and control

    std::atomic_size_t                  _jobsDone   = 0;                        // Counts jobs done until destroyed
    bool                                _shutdown   = false;                    // Control shutdown
    bool                                _paused     = false;                    // Control pause

    std::vector<std::exception_ptr>     _exceptions;                            // Buffer for exceptions
    std::mutex                          _exceptionsMtx;                         // Mutex for exception buffer

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

    thread_pool(const thread_pool& other)             = delete;
    thread_pool& operator=(const thread_pool& other)  = delete;

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

    // Stop the execution by swapping job queue with and empty queue
    // Worker threads wait until job queue has entries again
    void pause()
    {
        std::unique_lock<std::mutex> lock(_poolMtx);

        if (!_paused && !_shutdown)
        {
            _paused = true;
            std::swap(_validJobQueue, _idleJobQueue);
        }
        else
        {
            // do nothing - pool is already paused or shutdown
        }
    }

    // Resume the execution by swapping the empty queue with the original job queue
    // Worker threads are notified and can now get jobs
    void resume()   // has valid jobs again -> the worker can start
    {
        std::unique_lock<std::mutex> lock(_poolMtx);

        if (_paused && !_shutdown)
        {
            _paused = false;
            std::swap(_validJobQueue, _idleJobQueue);
            _poolCV.notify_all();
        }
        else
        {
            // do nothing - pool is already working or shutdown
        }
    }

    // Stop the execution and moves remaining jobs to idle queue
    // Clear thread vector and repopulate it with a new thread count
    // Start threads and move back the original job queue
    std::vector<std::exception_ptr> restart(const size_t nthreads)
    {
        std::vector<std::exception_ptr> ret;

        pause();    // no valid jobs

        _close_pool();  // threads join after running jobs are done

        std::swap(_exceptions, ret);

        _open_pool(nthreads);   // new threads are created and started

        resume();   // valid jobs are back

        return ret;
    }

    // Destroy threads, but NOT before finishing ALL jobs
    // Return caught exceptions
    std::vector<std::exception_ptr> shutdown()
    {
        std::vector<std::exception_ptr> ret;

        resume();   // ensure the jobs are ALL valid

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

        pause();    // no valid jobs

        _close_pool();  // threads join after running jobs are done

        std::swap(_exceptions, ret);

        return ret;
    }

    // Assign new job and priority
    void do_job(std::function<void(void)>&& newJob, job_priority prio = job_priority::normal)
    {
        // Move a job on the queue and unblock a thread
        std::unique_lock<std::mutex> lock(_poolMtx);

        // No longer accepting jobs
        if (_shutdown)
            return;

        // Can accept jobs
        if (_paused)
            _idleJobQueue.emplace(prio, std::move(newJob));
        else
        {
            _validJobQueue.emplace(prio, std::move(newJob));
            _poolCV.notify_one();
        }
    }

    // template<class JobContainer, _EnableJobsFromContainer_t<JobContainer> = true>
    // void do_more_jobs(JobContainer&& newJobs, job_priority prio = job_priority::normal)
    // {
    //     // Move multiple jobs on the queue and unblock necessary threads
    //     std::unique_lock<std::mutex> lock(_poolMtx);

    //     for (auto&& job : newJobs)
    //         _validJobQueue.emplace(prio, std::move(job));

    //     _poolCV.notify_all();
    // }

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

    // Destroy threads after jobs in valid queue are done
    void _close_pool()
    {
        // Notify all threads to stop waiting for job
        {
            std::unique_lock<std::mutex> lock(_poolMtx);

            _shutdown = true;
            _poolCV.notify_all();
        }

        // Threads will exit the loop and will join here
        for (auto& t : _workerThreads)
            t.join();

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

                // Pool is working, but there are no jobs -> wait
                // Safeguard against spurious wakeups (while instead of if)
                while (!_shutdown && _validJobQueue.empty())
                    _poolCV.wait(lock);

                // Pool is closed
                // Threads finish jobs first
                if (_validJobQueue.empty())
                    return;

                // Pool is working and jobs are available
                // Assign job to thread from queue
                auto it = _validJobQueue.begin();
                job = std::move(it->second);
                _validJobQueue.erase(it);
            }   // Empty scope end -> unlock, can start job


            // Do the job without holding any locks
            // Store exceptions
            try
            {
                job();
            }
            catch (const std::exception&)
            {
                std::unique_lock<std::mutex> lock(_exceptionsMtx);
                _exceptions.push_back(std::current_exception());
            }
            catch (...)
            {
                std::unique_lock<std::mutex> lock(_exceptionsMtx);
                std::runtime_error e("Job threw an unknown exception");
                _exceptions.push_back(std::make_exception_ptr(e));
            }

            // Count work done (even if throws)
            ++_jobsDone;
        }
    }

public:

    // template<class JobContainer, _EnableJobsFromContainer_t<JobContainer> = true>
    // static void start_and_display(JobContainer&& newJobs)
    // {
    //     static constexpr size_t barWidth = 50;

    //     size_t total    = newJobs.size();
    //     size_t current  = 0;
    //     size_t position = 0;
    //     float progress  = 0;

    //     {   // empty scope start -> to control the lifecycle of the pool
    //         thread_pool pool;
    //         pool.do_more_jobs(std::move(newJobs));

    //         // Start Display
    //         while (current < total)
    //         {
    //             current = pool.jobs_done();

    //             progress = static_cast<float>(current) / static_cast<float>(total);
    //             position = static_cast<size_t>(barWidth * progress);

    //             std::cout << "[";
    //             for (size_t i = 0; i < barWidth; ++i)
    //                 if (i < position) std::cout << "=";
    //                 else if (i == position) std::cout << ">";
    //                 else std::cout << " ";
    //             std::cout << "] " << static_cast<size_t>(progress * 100.0f) << " %\r";

    //             std::this_thread::sleep_for(std::chrono::milliseconds(100));
    //         }

    //         // End Display
    //         std::cout << '\n';
    //     }   // empty scope end -> thread_pool obj is destroyed, but waits for the threads to finish and join
    // }
};  // END thread_pool
