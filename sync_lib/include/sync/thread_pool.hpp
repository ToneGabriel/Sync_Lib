#pragma once

#include <thread>
#include <vector>

#include "sync/detail/scheduler.hpp"
#include "sync/execution_context.hpp"


SYNC_BEGIN


class thread_pool : public execution_context
{
private:

    // Task basic_executor object 
    detail::scheduler _scheduler;

    // Dynamic container for threads. Also join automatically when destroyed
    std::vector<std::jthread> _threads;

public:

    thread_pool()
        : thread_pool(std::thread::hardware_concurrency()) { /* Empty */ }

    thread_pool(size_t nthreads);

    ~thread_pool() override;

public:

    basic_executor& get_executor() override;

    size_t thread_count() const;

    size_t jobs_done() const;

    void stop();

    void join();
};  // END thread_pool


SYNC_END


#include "sync/detail/impl/thread_pool.ipp"
