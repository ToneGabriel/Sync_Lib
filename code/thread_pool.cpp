#include <thread_pool.h>


thread_pool::thread_pool()
{
    _open_pool(2);
}

thread_pool::thread_pool(const size_t nthreads)
{
    _open_pool(nthreads);
}

thread_pool::~thread_pool()
{
    _close_pool();
}

size_t thread_pool::size() const
{
    return _workerThreads.size();
}

void thread_pool::increase_size(const size_t nthreads)
{
    for (size_t i = 0; i < nthreads; ++i)
    {
        std::thread t = std::thread(std::bind(&thread_pool::_worker_thread, this));
        _workerThreads.push_back(std::move(t));
    }
}

void thread_pool::do_job(std::function<void()>&& newJob)
{
    // Move a job on the queue and unblock a thread
    std::unique_lock<std::mutex> lock(_poolMtx);

    _jobs.emplace(std::move(newJob));
    _poolCV.notify_one();
}

size_t thread_pool::jobs_done() const
{
    return _jobsDone;
}

void thread_pool::_open_pool(const size_t nthreads)
{
    _jobsDone = 0;
    _shutdown = false;

    increase_size(nthreads);
}

void thread_pool::_close_pool()
{
    // Notify all threads to stop waiting for job
    {
        std::unique_lock<std::mutex> lock(_poolMtx);

        _shutdown = true;
        _poolCV.notify_all();
    }

    // Threads will exit the loop and will join here
    for (auto& t : _workerThreads)
        t.join();

    // Print errors last
    while(!_errors.empty())
    {
        std::cout << "Error: " << _errors.front() << '\n';
        _errors.pop();
    }
}

void thread_pool::_worker_thread()
{
    std::function<void()> job;

    for (;;)
    {
        {   // empty scope start -> mutex lock and job decision
            std::unique_lock<std::mutex> lock(_poolMtx);

            // If the pool is still working, but there are no jobs -> wait
            // Safeguard against spurious wakeups
            while (!_shutdown && _jobs.empty())
                _poolCV.wait(lock);

            // True only when the pool is closed -> exit loop and this thread is joined in _close_pool
            if (_jobs.empty())
                return;

            // Assign job to thread from queue
            job = std::move(_jobs.front());
            _jobs.pop();
        }   // empty scope end -> unlock, can start job


        // Do the job without holding any locks
        try
        {
            job();
        }
        catch (const std::exception& e)
        {
            std::unique_lock<std::mutex> lock(_errorMtx);
            _errors.emplace(e.what());
        }
        catch (...)
        {
            std::unique_lock<std::mutex> lock(_errorMtx);
            _errors.emplace("Job threw an unknown exception.");
        }

        // Count work done
        ++_jobsDone;
    }
}