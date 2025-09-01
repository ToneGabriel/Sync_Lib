#ifndef SYNC_DETAIL_IMPL_OUTPUT_STREAM_IPP
#define SYNC_DETAIL_IMPL_OUTPUT_STREAM_IPP

#include "sync/detail/output_stream.hpp"

#include <type_traits>

SYNC_BEGIN
DETAIL_BEGIN

template<class OStreamType, class = void>
struct _HasOStreamInterface : std::false_type {};


template<class OStreamType>
struct _HasOStreamInterface<OStreamType,
                            std::void_t<decltype(std::declval<OStreamType>().good()),
                                        decltype(std::declval<OStreamType>().flush()),
                                        decltype(std::declval<OStreamType>().write(std::declval<const char*>(), std::declval<std::streamsize>()))
                                        >
                            > : std::true_type {};


template<class OStreamType>
constexpr bool _HasOStreamInterface_v = _HasOStreamInterface<OStreamType>::value;


template<class OStreamType>
bool output_stream_impl<OStreamType>::good() const noexcept
{
    return _ostreamRef.get().good();
}


template<class OStreamType>
void output_stream_impl<OStreamType>::write(const char* c, std::streamsize n)
{
    (void)_ostreamRef.get().write(c, n);
}


template<class OStreamType>
void output_stream_impl<OStreamType>::flush()
{
    (void)_ostreamRef.get().flush();
}


// =============================================================================================


bool output_stream::good() const
{
    if (!_storage)
        throw bad_output_stream();

    return _storage->good();
}


output_stream& output_stream::flush()
{
    if (!_storage)
        throw bad_output_stream();

    _storage->flush();
    return *this;
}


output_stream& output_stream::write(const char* c, std::streamsize n)
{
    if (!_storage)
        throw bad_output_stream();

    _storage->write(c, n);
    return *this;
}


template<class OStreamType>
void output_stream::_reset(OStreamType&& ostream)
{
    if constexpr (_HasOStreamInterface_v<std::decay_t<OStreamType>>)
    {
        using _OtherImpl = output_stream_impl<std::decay_t<OStreamType>>;
        _storage = std::make_unique<_OtherImpl>(ostream);
    }
}


DETAIL_END
SYNC_END


#endif  // SYNC_DETAIL_IMPL_OUTPUT_STREAM_IPP