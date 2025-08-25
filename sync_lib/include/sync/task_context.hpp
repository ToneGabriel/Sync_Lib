#pragma once


#include "sync/detail/scheduler.hpp"
#include "sync/execution_context.hpp"


SYNC_BEGIN


class task_context : public execution_context
{
private:

    // Basic executor for tasks
    detail::scheduler _scheduler;

public:

    task_context() = default;

    ~task_context() override = default;

public:

    /**
     * @brief Return a reference to the executor associated with the pool
     */
    basic_executor& get_executor() override;

    /**
     * @brief Returns `true` if the executor is stopped, `false` otherwise.
     */
    bool stopped() const;

    /**
     * @brief Allow new calls for `run()`
     */
    void restart();

    /**
     * @brief Start tasks
     */
    void run();

    /**
     * @brief Stop the executor. Pending jobs are no longer available.
     * Running jobs will continue.
     * Subsequent `run()` calls return immediately.
     */
    void stop();
};  // END task_context


SYNC_END


#include "sync/detail/impl/task_context.ipp"
