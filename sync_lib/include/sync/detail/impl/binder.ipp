#ifndef SYNC_DETAIL_IMPL_BINDER_IPP
#define SYNC_DETAIL_IMPL_BINDER_IPP

#include "sync/detail/binder.hpp"

SYNC_BEGIN
DETAIL_BEGIN


template<class Functor, class... Args>
void binder<Functor, Args...>::operator()(void) noexcept(noexcept(std::apply(_functor, _boundArgs)))
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


template<class Functor, class... Args>
std::future<typename binder<Functor, Args...>::return_type> binder<Functor, Args...>::get_future()
{
    return _promise.get_future();
}


DETAIL_END
SYNC_END

#endif  // SYNC_DETAIL_IMPL_BINDER_IPP