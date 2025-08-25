#include "sync/task_context.hpp"


SYNC_BEGIN


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
    _scheduler.stop_now();
}


SYNC_END