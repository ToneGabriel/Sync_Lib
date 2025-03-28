#pragma once

#include <functional>
#include <future>

#include "_sync_core.h"


SYNC_BEGIN

/**
 * @brief Class to store a functor and its arguments
 * @tparam Functor type of the function object
 * @tparam Args types of the arguments of the function object
 */
template<class Functor, class... Args>
class binder
{
public:
    /**
     * @brief Type of the return value from call
     */
    using return_type = std::invoke_result_t<Functor, Args...>;

private:
    /**
     * @brief Stored functor
     */
    std::decay_t<Functor> _functor;

    /**
     * @brief Stored arguments as tuple
     */
    std::tuple<std::decay_t<Args>...> _boundArgs;

    /**
     * @brief Promise to get future later with the result
     */
    std::promise<return_type> _promise;

public:
    // Constructors

    /**
     * @brief Delete default constructor. Bind only
     */
    binder() = delete;

    /**
     * @brief Destroy the binder object
     */
    ~binder() = default;

    /**
     * @brief Construct a new binder object by forwarding functor and arguments
     * @param func functio object to be called
     * @param args arguments for the call
     */
    explicit binder(Functor&& func, Args&&... args)
        :   _functor(std::forward<Functor>(func)),
            _boundArgs(std::forward<Args>(args)...) { /*Empty*/ }

    /**
     * @brief Perform call `func(args...)`
     * @note Call `get_future()` to get the `std::future` object for call result
     */
    void operator()(void) noexcept(noexcept(std::apply(_functor, _boundArgs)))
    {
        try
        {
            if constexpr (std::is_void_v<return_type>)
            {
                std::apply(_functor, _boundArgs);
                _promise.set_value();
            }
            else
                _promise.set_value(std::apply(_functor, _boundArgs));
        }
        catch(...)
        {
            _promise.set_exception(std::current_exception());
        }
    }

    /**
     * @brief Get the future object
     * @return `std::future<return_type>`
     */
    std::future<return_type> get_future()
    {
        return _promise.get_future();
    }
};  // END binder

SYNC_END
