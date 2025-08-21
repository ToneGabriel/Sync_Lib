#pragma once

#include "sync/detail/scheduler.hpp"
#include "sync/execution_context.hpp"


SYNC_BEGIN


class thread_pool : public execution_context
{
private:

    detail::scheduler _scheduler;

public:

    thread_pool() = default;
    ~thread_pool() override;

public:

    basic_executor& get_executor() override;
};  // END thread_pool


SYNC_END


#include "sync/detail/impl/thread_pool.ipp"
