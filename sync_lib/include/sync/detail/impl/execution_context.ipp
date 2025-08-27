#ifndef SYNC_DETAIL_IMPL_EXECUTION_CONTEXT_IPP
#define SYNC_DETAIL_IMPL_EXECUTION_CONTEXT_IPP

#include "sync/execution_context.hpp"

SYNC_BEGIN

template<class Functor, class... Args>
std::future<std::invoke_result_t<Functor, Args...>> post(execution_context& context, priority prio, Functor&& func, Args&&... args)
{
    basic_executor& executor = context.get_executor();

    if (executor.stopped())
        throw std::system_error(std::make_error_code(std::errc::operation_not_permitted), "Context executor is stopped");

    auto taskPtr = std::make_shared<detail::binder<Functor, Args...>>
                        (std::forward<Functor>(func), std::forward<Args>(args)...);

    executor.post(detail::priority_job(prio, [taskPtr](void) { return (*taskPtr)(); }));

    // Return the future of the job's result
    return taskPtr->get_future();
}


template<class Functor, class... Args>
std::future<std::invoke_result_t<Functor, Args...>> post(execution_context& context, Functor&& func, Args&&... args)
{
    return post(context, priority::medium, std::forward<Functor>(func), std::forward<Args>(args)...);
}

SYNC_END


#endif  // SYNC_DETAIL_IMPL_EXECUTION_CONTEXT_IPP