#ifndef SYNC_BASIC_EXECUTOR_HPP
#define SYNC_BASIC_EXECUTOR_HPP

#include <memory>

#include "sync/detail/priority_job.hpp"
#include "sync/detail/binder.hpp"


SYNC_BEGIN


class basic_executor
{
public:
    virtual ~basic_executor() = default;
    virtual void post(detail::priority_job&& job) = 0;
    virtual bool stopped() const = 0;
};  // END basic_executor


SYNC_END

#endif  // SYNC_BASIC_EXECUTOR_HPP
