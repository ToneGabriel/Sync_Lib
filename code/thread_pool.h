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


enum class priority
{
    lowest,
    low,
    normal,
    high,
    highest
};  // END priority


class thread_pool
{
private:

    class priority_job;

private:
    // std::priority_queue<priority_job>       _jobs;
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

    thread_pool();

    thread_pool(const size_t nthreads);

    ~thread_pool();

    thread_pool(const thread_pool& other)             = delete;
    thread_pool& operator=(const thread_pool& other)  = delete;

public:
    // Main functions

    size_t size() const;
    void increase_size(const size_t nthreads);

    size_t jobs_done() const;
    void do_job(std::function<void(void)>&& newJob);

    template<class JobContainer, _EnableJobsFromContainer_t<JobContainer> = true>
    void do_more_jobs(JobContainer&& newJobs);

private:
    // Helpers

    void _open_pool(const size_t nthreads);

    void _close_pool();

    void _worker_thread();

public:

    template<class JobContainer, _EnableJobsFromContainer_t<JobContainer> = true>
    static void start_and_display(JobContainer&& newJobs);
};  // END thread_pool



template<class JobContainer, _EnableJobsFromContainer_t<JobContainer> = true>
void thread_pool::do_more_jobs(JobContainer&& newJobs)
{
    // Move multiple jobs on the queue and unblock necessary threads
    std::unique_lock<std::mutex> lock(_poolMtx);

    for (auto&& job : newJobs)
    {
        _jobs.emplace(std::move(job));
        _poolCV.notify_one();
    }
}

template<class JobContainer, _EnableJobsFromContainer_t<JobContainer> = true>
void thread_pool::start_and_display(JobContainer&& newJobs)
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



class thread_pool::priority_job
{
private:
    std::function<void(void)>   _job;
    priority                    _prio;

public:

    priority_job(std::function<void(void)>&& job, priority prio)
        : _job(std::move(job)), _prio(prio) { /* Empty */ }
};  // END thread_pool::priority_job
