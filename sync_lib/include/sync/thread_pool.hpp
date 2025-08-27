#ifndef SYNC_THREAD_POOL_HPP
#define SYNC_THREAD_POOL_HPP

#include <thread>
#include <vector>

#include "sync/detail/scheduler.hpp"
#include "sync/execution_context.hpp"


SYNC_BEGIN


/**
 * @brief The thread pool class is an execution context where functions are permitted to run on one of a fixed number of threads.
 * 
 * Use `sync::post()` to submit functions to the pool.
 */
class thread_pool : public execution_context
{
private:

    // Basic executor for tasks
    detail::scheduler _scheduler;

    // Dynamic container for threads. Also join automatically when destroyed
    std::vector<std::jthread> _threads;

public:

    /**
     * @brief Construct thread_pool with default number of threads `std::thread::hardware_concurrency()`
     */
    SYNC_DECL thread_pool()
        : thread_pool(std::thread::hardware_concurrency()) { /* Empty */ }

    /**
     * @brief Construct thread_pool with specified number of threads
     * @param nthreads number of threads
     */
    SYNC_DECL thread_pool(size_t nthreads);

    /**
     * @brief Calls `join()` before destroying the object
     */
    SYNC_DECL ~thread_pool() override;

public:

    /**
     * @brief Return a reference to the executor associated with the pool
     */
    SYNC_DECL basic_executor& get_executor() override;

    /**
     * @brief Return the number of running threads
     */
    SYNC_DECL size_t thread_count() const;

    /**
     * @brief Return the number of tasks finished (even if they throw)
     */
    SYNC_DECL size_t jobs_done() const;

    /**
     * @brief Returns `true` if the executor is stopped, `false` otherwise.
     */
    SYNC_DECL bool stopped() const;

    /**
     * @brief Stop the executor (non-blocking). Pending jobs are no longer available.
     * Running jobs will continue.
     */
    SYNC_DECL void stop();

    /**
     * @brief Block until all pending jobs are finished, then join threads.
     */
    SYNC_DECL void join();
};  // END thread_pool


SYNC_END

#ifdef SYNC_HEADER_ONLY
#   include "sync/detail/impl/thread_pool.ipp"
#endif  // SYNC_HEADER_ONLY

#endif  // SYNC_THREAD_POOL_HPP
