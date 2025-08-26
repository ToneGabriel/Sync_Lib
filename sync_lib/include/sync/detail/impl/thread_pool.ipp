#pragma once

#include "sync/thread_pool.hpp"


SYNC_BEGIN


thread_pool::thread_pool(size_t nthreads)
{
    _SYNC_ASSERT(nthreads > 0, "Pool cannot have 0 threads!");

    _scheduler.restart();
    _threads.reserve(nthreads);

    while (nthreads--)
    {
        std::jthread t(&detail::scheduler::run, &_scheduler);
        _threads.push_back(std::move(t));
    }
}


thread_pool::~thread_pool()
{
    join();
}


basic_executor& thread_pool::get_executor()
{
    return _scheduler;
}


size_t thread_pool::thread_count() const
{
    return _threads.size();
}


size_t thread_pool::jobs_done() const
{
    return _scheduler.jobs_done();
}


bool thread_pool::stopped() const
{
    return _scheduler.stopped();
}


void thread_pool::stop()
{
    _scheduler.stop_now();
}


void thread_pool::join()
{
    _scheduler.stop();
    _threads.clear();
}


SYNC_END