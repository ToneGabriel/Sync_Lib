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
    // Priority queue for tasks
    std::priority_queue<detail::priority_job> _pendingJobs;

    // Safety mutex
    mutable std::mutex _pendingJobsMtx;

    // Condition variable for empty queue wait
    mutable std::condition_variable _pendingJobsCV;

    // Finished tasks counter
    std::atomic_size_t _jobsDone = 0;

    // Flag used for stop state
    bool _stop = false;

public:

    scheduler() = default;

    ~scheduler() override;

public:

    /**
     * @brief Used internally by `sync::post()` to submit tasks
     */
    void post(detail::priority_job&& job) override;

    /**
     * @brief Returns `true` if the executor is stopped, `false` otherwise.
     */
    bool stopped() const override;

    /**
     * @brief Return the number of tasks finished (even if they throw)
     */
    size_t jobs_done() const;

    /**
     * @brief Stop the executor. Pending jobs finish before return.
     * Subsequent `run()` calls return immediately.
     */
    void stop();

    /**
     * @brief Stop the executor. Pending jobs are no longer available.
     * Running jobs will continue.
     * Subsequent `run()` calls return immediately.
     */
    void stop_now();

    /**
     * @brief Allow new calls for `run()`
     */
    void restart();

    /**
     * @brief Start tasks
     */
    void run();
};  // END scheduler


DETAIL_END
SYNC_END


#include "sync/detail/impl/scheduler.ipp"
