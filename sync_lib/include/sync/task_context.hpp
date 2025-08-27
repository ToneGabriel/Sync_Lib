#ifndef SYNC_TASK_CONTEXT_HPP
#define SYNC_TASK_CONTEXT_HPP


#include "sync/detail/scheduler.hpp"
#include "sync/execution_context.hpp"


SYNC_BEGIN


class task_context : public execution_context
{
private:

    // Basic executor for tasks
    detail::scheduler _scheduler;

public:

    SYNC_DECL task_context() = default;

    SYNC_DECL ~task_context() override = default;

public:

    /**
     * @brief Return a reference to the executor associated with the pool
     */
    SYNC_DECL basic_executor& get_executor() override;

    /**
     * @brief Returns `true` if the executor is stopped, `false` otherwise.
     */
    SYNC_DECL bool stopped() const;

    /**
     * @brief Allow new calls for `run()`
     */
    SYNC_DECL void restart();

    /**
     * @brief Start tasks
     */
    SYNC_DECL void run();

    /**
     * @brief Stop the executor. Pending jobs are no longer available.
     * Running jobs will continue.
     * Subsequent `run()` calls return immediately.
     */
    SYNC_DECL void stop();
};  // END task_context


SYNC_END

#ifdef SYNC_HEADER_ONLY
#   include "sync/detail/impl/task_context.ipp"
#endif  // SYNC_HEADER_ONLY

#endif  // SYNC_TASK_CONTEXT_HPP
