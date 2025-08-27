#ifndef SYNC_EXECUTION_CONTEXT_HPP
#define SYNC_EXECUTION_CONTEXT_HPP

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
std::future<std::invoke_result_t<Functor, Args...>> post(execution_context& context, priority prio, Functor&& func, Args&&... args);


/**
 * @brief Overloaded variant with medium priority
 */
template<class Functor, class... Args>
std::future<std::invoke_result_t<Functor, Args...>> post(execution_context& context, Functor&& func, Args&&... args);


SYNC_END

#include "sync/detail/impl/execution_context.ipp"

#endif  // SYNC_EXECUTION_CONTEXT_HPP
