#ifndef SYNC_DETAIL_IMPL_PRIORITY_JOB_IPP
#define SYNC_DETAIL_IMPL_PRIORITY_JOB_IPP

#include "sync/detail/priority_job.hpp"


SYNC_BEGIN
DETAIL_BEGIN


priority_job::priority_job(priority_job&& other) noexcept
{
    _move(std::move(other));
}


priority_job& priority_job::operator=(priority_job&& other) noexcept
{
    if (this != &other)
        _move(std::move(other));

    return *this;
}


void priority_job::operator()(void) const
{
    _job();
}


uint8_t priority_job::effective_priority() const
{
    // Subtract from original priority the number of seconds this object waited since timestamp
    auto age    = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - _timestamp).count();
    auto prio   = static_cast<uint8_t>(_prio);

    return (prio <= age) ? 0 : prio - age;
}


void priority_job::_move(priority_job&& other) noexcept
{
    _prio       = other._prio;
    _job        = std::move(other._job);
    _timestamp  = std::chrono::steady_clock::now(); // timestamp is refreshed
}


DETAIL_END
SYNC_END


#endif  // SYNC_DETAIL_IMPL_PRIORITY_JOB_IPP
