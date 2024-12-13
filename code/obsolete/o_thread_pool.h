template<class Container, class = void>
struct _HasValidMethods : std::false_type {};

template<class Container>
struct _HasValidMethods<Container, std::void_t< decltype(std::declval<Container>().size()),
                                                decltype(std::declval<Container>().begin()),
                                                decltype(std::declval<Container>().end())>> : std::true_type {};

template<class Container>
using _EnableJobsFromContainer_t =
            std::enable_if_t<std::conjunction_v<
                                    std::is_same<typename Container::value_type, std::function<void(void)>>,
                                    std::is_convertible<typename std::iterator_traits<typename Container::iterator>::iterator_category, std::forward_iterator_tag>,
                                    _HasValidMethods<Container>>,
            bool>;



template<class JobContainer, _EnableJobsFromContainer_t<JobContainer> = true>
void do_more_jobs(JobContainer&& newJobs, job_priority prio = job_priority::normal)
{
    Move multiple jobs on the queue and unblock necessary threads
    std::unique_lock<std::mutex> lock(_poolMtx);

    for (auto&& job : newJobs)
        _validJobQueue.emplace(prio, std::move(job));

    _poolCV.notify_all();
}



template<class JobContainer, _EnableJobsFromContainer_t<JobContainer> = true>
static void start_and_display(JobContainer&& newJobs)
{
    static constexpr size_t barWidth = 50;

    size_t total    = newJobs.size();
    size_t current  = 0;
    size_t position = 0;
    float progress  = 0;

    {   // empty scope start -> to control the lifecycle of the pool
        thread_pool pool;
        pool.do_more_jobs(std::move(newJobs));

        Start Display
        while (current < total)
        {
            current = pool.jobs_done();

            progress = static_cast<float>(current) / static_cast<float>(total);
            position = static_cast<size_t>(barWidth * progress);

            std::cout << "[";
            for (size_t i = 0; i < barWidth; ++i)
                if (i < position) std::cout << "=";
                else if (i == position) std::cout << ">";
                else std::cout << " ";
            std::cout << "] " << static_cast<size_t>(progress * 100.0f) << " %\r";

            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        End Display
        std::cout << '\n';
    }   // empty scope end -> thread_pool obj is destroyed, but waits for the threads to finish and join
}