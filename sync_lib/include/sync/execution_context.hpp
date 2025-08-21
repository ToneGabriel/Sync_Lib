#pragma once

#include "sync/basic_executor.hpp"


SYNC_BEGIN


class execution_context
{
public:
    virtual ~execution_context() = default;
    virtual basic_executor& get_executor() = 0;
};  // END execution_context


template<class Functor, class... Args>
std::future<std::invoke_result_t<Functor, Args...>> post(execution_context& context, priority prio, Functor&& func, Args&&... args)
{
    auto taskPtr = std::make_shared<detail::binder<Functor, Args...>>
                        (std::forward<Functor>(func), std::forward<Args>(args)...);

    context.get_executor().post(detail::priority_job(prio, [taskPtr](void) { return (*taskPtr)(); }));

    // Return the future of the job's result
    return taskPtr->get_future();
}


template<class Functor, class... Args>
std::future<std::invoke_result_t<Functor, Args...>> post(execution_context& context, Functor&& func, Args&&... args)
{
    return post(context, priority::normal, std::forward<Functor>(func), std::forward<Args>(args)...);
}


SYNC_END