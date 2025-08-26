#pragma once

#include "sync/basic_executor.hpp"


SYNC_BEGIN


class execution_context
{
public:
    virtual ~execution_context() = default;
    virtual basic_executor& get_executor() = 0;
};  // END execution_context


/**
 * @brief Submit tasks to an execution context
 * @param context Execution context where the task is executed
 * @param prio Optional: Priority for scheduling
 * @param func Task to execute
 * @param args Arguments for task execution
 * @return A `std::future` of the task result
 * @throw `std::system_error` if the context executor is stopped
 */
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


/**
 * @brief Overloaded variant with medium priority
 */
template<class Functor, class... Args>
std::future<std::invoke_result_t<Functor, Args...>> post(execution_context& context, Functor&& func, Args&&... args)
{
    return post(context, priority::medium, std::forward<Functor>(func), std::forward<Args>(args)...);
}


SYNC_END