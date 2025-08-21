#include "sync/thread_pool.hpp"


SYNC_BEGIN


thread_pool::~thread_pool()
{
    // TODO
}


basic_executor& thread_pool::get_executor()
{
    return _scheduler;
}


SYNC_END