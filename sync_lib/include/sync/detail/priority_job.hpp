#ifndef SYNC_DETAIL_PRIORITY_JOB_HPP
#define SYNC_DETAIL_PRIORITY_JOB_HPP

#include <chrono>
#include <functional>

#include "sync/detail/core.hpp"


SYNC_BEGIN


/**
 * @brief Enum for job priority in thread_pool internal queue
 * @note Priority might change based on wait time
 */
enum class priority : uint8_t
{
    highest = 0,
    high    = UINT8_MAX / 4,
    medium  = UINT8_MAX / 2,
    low     = UINT8_MAX / 4 * 3,
    lowest  = UINT8_MAX
};  // END priority


DETAIL_BEGIN


/**
 * @brief Helper class to be integrated into a priority queue
 */
class priority_job
{
private:

    // User set priority
    priority _prio;

    // The actual job
    std::function<void(void)> _job;

    // Insertion time
    typename std::chrono::steady_clock::time_point _timestamp;

public:

    priority_job() = default;
    ~priority_job() = default;

    /**
     * @brief Constructor that sets priority and job for this object
     * @note Job ownership is transfered
     */
    SYNC_DECL priority_job(priority prio, std::function<void(void)>&& job)
        :   _prio(prio),
            _job(std::move(job)),
            _timestamp(std::chrono::steady_clock::now()) { /* Empty */ }

    /**
     * @brief Delete copy constructor and operator
     */
    priority_job(const priority_job&)             = delete;
    priority_job& operator=(const priority_job&)  = delete;

    SYNC_DECL priority_job(priority_job&& other) noexcept;
    SYNC_DECL priority_job& operator=(priority_job&& other) noexcept;

public:

    /**
     * @brief Call the stored job if any
     */
    SYNC_DECL void operator()(void) const;

    /**
     * @brief Number used by `operator<` for comparison in `std::priority_queue`
     * @note Priority might be higher than the original due to wait time
     */
    SYNC_DECL uint8_t effective_priority() const;

private:

    /**
     * @brief Ownership transfer algorithm
     * @param other object from where to get data
     */
    SYNC_DECL void _move(priority_job&& other) noexcept;
};  // END priority_job


/**
 * @brief Operator used in `std::priority_queue` ordering
 * @param left, right object to compare
 * @note Lower numbers mean higher priority
 */
inline bool operator<(const priority_job& left, const priority_job& right)
{
    return left.effective_priority() > right.effective_priority();
}


DETAIL_END
SYNC_END

#ifdef SYNC_HEADER_ONLY
#   include "sync/detail/impl/priority_job.ipp"
#endif  // SYNC_HEADER_ONLY

#endif  // SYNC_DETAIL_PRIORITY_JOB_HPP
