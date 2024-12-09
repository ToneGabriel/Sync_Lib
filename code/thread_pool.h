#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <queue>
#include <string>
#include <vector>
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


class thread_pool
{
private:
    std::queue<std::function<void(void)>>   _jobs;
    std::vector<std::thread>                _workerThreads;
    std::mutex                              _poolMtx;
    std::condition_variable                 _poolCV;

    std::atomic_size_t                      _jobsDone;
    bool                                    _shutdown;

    std::queue<std::string>                 _errors;
    std::mutex                              _errorMtx;

public:
    // Constructors & Operators

    thread_pool(const size_t nthreads = 2)
    {
        _jobsDone = 0;
        _shutdown = false;

        increase_size(nthreads);
    }

    ~thread_pool()
    {
        // Notify all threads to stop waiting for job
        {   // empty scope start -> lock
            std::unique_lock<std::mutex> lock(_poolMtx);

            _shutdown = true;
            _poolCV.notify_all();
        }   // empty scope end -> unlock

        // Threads will exit the loop and will join here
        for (auto& t : _workerThreads)
            t.join();

        // Print errors last
        while(!_errors.empty())
        {
            std::cout << "Error: " << _errors.front() << '\n';
            _errors.pop();
        }
    }

    thread_pool(const thread_pool& other)             = delete;
    thread_pool& operator=(const thread_pool& other)  = delete;

public:
    // Main functions

    size_t size() const
    {
        return _workerThreads.size();
    }

    void increase_size(const size_t nthreads)
    {
        for (size_t i = 0; i < nthreads; ++i)
        {
            std::thread t = std::thread(std::bind(&thread_pool::_worker_thread, this));
            _workerThreads.push_back(std::move(t));
        }
    }

    void do_job(std::function<void()>&& newJob)
    {
        // Move a job on the queue and unblock a thread
        std::unique_lock<std::mutex> lock(_poolMtx);

        _jobs.emplace(std::move(newJob));
        _poolCV.notify_one();
    }

    template<class JobContainer, _EnableJobsFromContainer_t<JobContainer> = true>
    void do_more_jobs(JobContainer&& newJobs)
    {
        // Move multiple jobs on the queue and unblock necessary threads
        std::unique_lock<std::mutex> lock(_poolMtx);

        for (auto&& job : newJobs)
        {
            _jobs.emplace(std::move(job));
            _poolCV.notify_one();
        }
    }

    size_t jobs_done() const
    {
        return _jobsDone;
    }

private:
    // Helpers

    void _worker_thread()
    {
        std::function<void()> job;

        for (;;)
        {
            {   // empty scope start -> mutex lock and job decision
                std::unique_lock<std::mutex> lock(_poolMtx);

                // If the pool is still working, but there are no jobs -> wait
                // Safeguard against spurious wakeups
                while (!_shutdown && _jobs.empty())
                    _poolCV.wait(lock);

                // True only when the pool is closed -> exit loop and this thread is joined
                if (_jobs.empty())
                    return;

                // Assign job to thread from queue
                job = std::move(_jobs.front());
                _jobs.pop();
            }   // empty scope end -> unlock, can start job


            // Do the job without holding any locks
            try
            {
                job();
            }
            catch (const std::exception& e)
            {
                std::unique_lock<std::mutex> lock(_errorMtx);
                _errors.emplace(e.what());
            }
            catch (...)
            {
                std::unique_lock<std::mutex> lock(_errorMtx);
                _errors.emplace("Job threw an unknown exception.");
            }

            // Count work done
            ++_jobsDone;
        }
    }

public:

    template<class JobContainer, _EnableJobsFromContainer_t<JobContainer> = true>
    static void start_and_display(JobContainer&& newJobs)
    {
        const size_t barWidth = 50;

        size_t total    = newJobs.size();
        size_t current  = 0;
        size_t position = 0;
        float progress  = 0;

        {   // empty scope start -> to control the lifecycle of the pool
            thread_pool pool;
            pool.do_more_jobs(std::move(newJobs));

            // Start Display
            while (current < total)
            {
                current = pool.jobs_done();
                
                progress = static_cast<float>(current) / static_cast<float>(total);
                position = static_cast<size_t>(barWidth * progress);

                std::cout << "[";
                for (size_t i = 0; i < barWidth; ++i)
                    if (i < position) std::cout << "=";
                    else if (i == position) std::cout << ">";
                    else std::cout << " ";
                std::cout << "] " << static_cast<size_t>(progress * 100.0f) << " %\r";

                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }

            // End Display
            std::cout << '\n';
        }   // empty scope end -> thread_pool obj is destroyed, but waits for the threads to finish and join
    }
};  // END thread_pool