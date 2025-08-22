#pragma once


#include "sync/detail/scheduler.hpp"
#include "sync/execution_context.hpp"


SYNC_BEGIN


class task_context : public execution_context
{
private:

    // Task basic_executor object 
    detail::scheduler _scheduler;

public:

    task_context() = default;

    ~task_context() override = default;

public:

    basic_executor& get_executor() override;

    bool stopped() const;

    void restart();

    void run();

    void stop();
};  // END task_context


SYNC_END


#include "sync/detail/impl/task_context.ipp"
