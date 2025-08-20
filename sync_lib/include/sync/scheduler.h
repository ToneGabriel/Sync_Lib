#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <future>
#include <queue>
#include <vector>

#include "sync/detail/core.h"


/**
 * @brief Enum for job priority in thread_pool internal queue
 * @note Priority might change based on wait time
 */
enum class priority : uint8_t
{
    highest = 0,
    high    = UINT8_MAX / 4,
    normal  = UINT8_MAX / 2,
    low     = UINT8_MAX / 4 * 3,
    lowest  = UINT8_MAX
};  // END priority


DETAIL_BEGIN


/**
 * @brief Helper class to be integrated into a priority queue
 */
class _priority_job
{
private:

    /**
     * @brief User set priority
     */
    priority _prio;

    /**
     * @brief The actual job
     */
    std::function<void(void)> _job;

    /**
     * @brief Insertion time
     */
    typename std::chrono::steady_clock::time_point _timestamp;

public:

    /**
     * @brief Default constructor
     */
    _priority_job() = default;

    /**
     * @brief Constructor that sets priority and job for this object
     * @note Job ownership is transfered
     */
    _priority_job(priority prio, std::function<void(void)>&& job)
        :   _prio(prio),
            _job(std::move(job)),
            _timestamp(std::chrono::steady_clock::now()) { /* Empty */ }

    /**
     * @brief Default destructor
     */
    ~_priority_job() = default;

    /**
     * @brief Delete copy constructor and operator
     */
    _priority_job(const _priority_job&)             = delete;
    _priority_job& operator=(const _priority_job&)  = delete;

    /**
     * @brief Move constructor
     * @param other object to be moved
     */
    _priority_job(_priority_job&& other) noexcept
    {
        _move(std::move(other));
    }

    /**
     * @brief Move assign operator
     * @param other object to be moved
     */
    _priority_job& operator=(_priority_job&& other) noexcept
    {
        if (this != &other)
            _move(std::move(other));

        return *this;
    }

public:

    /**
     * @brief Call the stored job if any
     */
    void operator()() const
    {
        if (_job)
            _job();
    }

    /**
     * @brief Number used by `operator<` for comparison in `std::priority_queue`
     * @note Priority might be higher than the original due to wait time
     */
    uint8_t effective_priority() const
    {
        // Subtract from original priority the number of seconds this object waited since timestamp
        auto age    = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - _timestamp).count();
        auto prio   = static_cast<uint8_t>(_prio);

        return (prio <= age) ? 0 : prio - age;
    }

private:

    /**
     * @brief Ownership transfer algorithm
     * @param other object from where to get data
     */
    void _move(_priority_job&& other) noexcept
    {
        _prio       = other._prio;
        _job        = std::move(other._job);
        _timestamp  = std::chrono::steady_clock::now(); // timestamp is refreshed
    }
};  // END _priority_job


/**
 * @brief Operator used in `std::priority_queue` ordering
 * @param left, right object to compare
 * @note Lower numbers mean higher priority
 */
inline bool operator<(const _priority_job& left, const _priority_job& right)
{
    return left.effective_priority() > right.effective_priority();
}


DETAIL_END


class scheduler
{
private:
    std::priority_queue<detail::_priority_job>  _pendingJobs;
    mutable std::mutex                          _pendingJobsMtx;
    mutable std::condition_variable             _pendingJobsCV;

public:
    scheduler() = default;

    void post();

    void run()
    {
        detail::_priority_job job;

        for (;;)
        {
            {   // Empty scope start -> mutex lock and job decision
                std::unique_lock<std::mutex> lock(_pendingJobsMtx);

                // Pool is working (not joined), but is paused or there are no jobs -> wait
                // Safeguard against spurious wakeups (while instead of if)
                while (!_get_join_flag() && _pendingJobs.empty())
                    _pendingJobsCV.wait(lock);

                // Pool is joined
                // Threads finish pending jobs first if not paused
                if (_pendingJobs.empty())
                    return;

                // Pool is working/joined, not paused and jobs are available
                // Assign job to thread from queue
                job = std::move(const_cast<detail::_priority_job&>(_pendingJobs.top()));
                _pendingJobs.pop();
            }   // Empty scope end -> unlock, can start job

            // Do the job without holding any locks
            job();

            // Count work done (even if throws)
            // _increment_jobs_done();
        }
    }

    void stop()
    {
        
    }

};  // END scheduler