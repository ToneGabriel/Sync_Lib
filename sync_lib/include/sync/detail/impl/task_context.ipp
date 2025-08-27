#ifndef SYNC_DETAIL_IMPL_TASK_CONTEXT_IPP
#define SYNC_DETAIL_IMPL_TASK_CONTEXT_IPP

#include "sync/task_context.hpp"


SYNC_BEGIN


task_context::task_context()
{
    _scheduler.restart();
    _scheduler.forbid_wait();
}


task_context::~task_context()
{
    // Empty - scheduler stops by default
}


basic_executor& task_context::get_executor()
{
    return _scheduler;
}


bool task_context::stopped() const
{
    return _scheduler.stopped();
}


void task_context::restart()
{
    _scheduler.restart();
}


void task_context::run()
{
    _scheduler.run();
}


void task_context::stop()
{
    _scheduler.stop();
}


SYNC_END


#endif  // SYNC_DETAIL_IMPL_TASK_CONTEXT_IPP
