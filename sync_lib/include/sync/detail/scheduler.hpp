#ifndef SYNC_DETAIL_SCHEDULER_HPP
#define SYNC_DETAIL_SCHEDULER_HPP

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


/**
 * @brief Basic task executor with priority ordering. `run()` can be called from multiple threads to execute pending jobs
 * @note If allowed to wait, not stopped and no pending jobs -> wait
 * @note If allowed to wait, stopped -> don't accept new jobs, execute all pending jobs
 * @note If not allowed to wait, not stopped -> accept new jobs, execute all pending jobs
 * @note If not allowed to wait, stopped -> don't accept new jobs, don't execute pending jobs
 */
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

    // Flag used to allow waiting for jobs
    bool _wait = false;

public:

    scheduler() = default;

    SYNC_DECL ~scheduler() override;

public:

    /**
     * @brief Used internally by `sync::post()` to submit tasks
     */
    SYNC_DECL void post(detail::priority_job&& job) override;

    /**
     * @brief Returns `true` if the executor is stopped, `false` otherwise.
     */
    SYNC_DECL bool stopped() const override;

    /**
     * @brief Return the number of tasks finished (even if they throw)
     */
    SYNC_DECL size_t jobs_done() const;

    /**
     * @brief Stop the executor. Pending jobs finish before return if `allowed_wait()` was called.
     * Running jobs will continue.
     * Subsequent `run()` calls return immediately.
     */
    SYNC_DECL void stop();

    /**
     * @brief Allow new calls for `run()`
     */
    SYNC_DECL void restart();

    /**
     * @brief Returns `true` if the executor can wait for new jobs if none pending, `false` otherwise.
     */
    SYNC_DECL bool allowed_to_wait() const;

    /**
     * @brief Threads executing `run()` are allowed to wait for new jobs if not stopped
     */
    SYNC_DECL void allow_wait();

    /**
     * @brief Threads executing `run()` exit if stopped or no jobs left
     */
    SYNC_DECL void forbid_wait();

    /**
     * @brief Start tasks
     */
    SYNC_DECL void run();
};  // END scheduler


DETAIL_END
SYNC_END

#ifdef SYNC_HEADER_ONLY
#   include "sync/detail/impl/scheduler.ipp"
#endif  // SYNC_HEADER_ONLY

#endif  // SYNC_DETAIL_SCHEDULER_HPP
