#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <future>
#include <queue>

#include "sync/detail/binder.hpp"
#include "sync/detail/priority_job.hpp"
#include "sync/basic_executor.hpp"


SYNC_BEGIN
DETAIL_BEGIN


class scheduler : public basic_executor
{
private:
    std::priority_queue<detail::priority_job> _pendingJobs;

    std::mutex _pendingJobsMtx;

    std::condition_variable _pendingJobsCV;

    std::atomic_size_t _jobsDone = 0;

    bool _stop = false;

public:

    scheduler() = default;

    ~scheduler() override;

public:

    void post(detail::priority_job&& job) override;

    size_t jobs_done() const;

    void stop();

    void stop_now();

    void restart();

    void run();
};  // END scheduler


DETAIL_END
SYNC_END


#include "sync/detail/impl/scheduler.ipp"
