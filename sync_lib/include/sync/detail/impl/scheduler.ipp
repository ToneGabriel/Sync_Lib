#ifndef SYNC_DETAIL_IMPL_SCHEDULER_IPP
#define SYNC_DETAIL_IMPL_SCHEDULER_IPP

#include "sync/detail/scheduler.hpp"


SYNC_BEGIN
DETAIL_BEGIN


scheduler::~scheduler()
{
    stop();
}


void scheduler::post(detail::priority_job&& job)
{
    std::lock_guard lock(_pendingJobsMtx);
    _pendingJobs.emplace(std::move(job));
    _pendingJobsCV.notify_one();
}


size_t scheduler::jobs_done() const
{
    return _jobsDone;
}


bool scheduler::stopped() const
{
    std::lock_guard lock(_pendingJobsMtx);
    return _stop;
}


void scheduler::stop()
{
    std::lock_guard lock(_pendingJobsMtx);
    _stop = true;
    _pendingJobsCV.notify_all();
}


void scheduler::restart()
{
    std::lock_guard lock(_pendingJobsMtx);
    _stop = false;
}


bool scheduler::allowed_to_wait() const
{
    std::lock_guard lock(_pendingJobsMtx);
    return _wait;
}


void scheduler::allow_wait()
{
    std::lock_guard lock(_pendingJobsMtx);
    _wait = true;
}


void scheduler::forbid_wait()
{
    std::lock_guard lock(_pendingJobsMtx);
    _wait = false;
    _pendingJobsCV.notify_all();
}


void scheduler::run()
{
    detail::priority_job job;

    for (;;)
    {
        {   // Empty scope start -> mutex lock and job decision
            std::unique_lock<std::mutex> lock(_pendingJobsMtx);

            if (_wait)  // Can wait for work if not stopped and queue is empty
            {
                // Pool is working (not stoped), but there are no jobs -> wait
                // Safeguard against spurious wakeups (while instead of if)
                while (!_stop && _pendingJobs.empty())
                    _pendingJobsCV.wait(lock);

                // Wait was disabled. Go to else branch
                if (!_wait)
                    continue;

                // Pool is stoped
                // Threads finish pending jobs first
                if (_pendingJobs.empty())
                    return;
            }
            else    // Exit if stopped or there are no jobs left
            {
                if (_stop || _pendingJobs.empty())
                    return;
            }

            job = std::move(const_cast<detail::priority_job&>(_pendingJobs.top()));
            _pendingJobs.pop();
        }   // Empty scope end -> unlock, can start job

        // Do the job without holding any locks
        job();

        // Count work done (even if throws)
        ++_jobsDone;
    }
}


DETAIL_END
SYNC_END


#endif  // SYNC_DETAIL_IMPL_SCHEDULER_IPP
