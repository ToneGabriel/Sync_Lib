#ifndef SYNC_DETAIL_IMPL_OUTPUT_STREAM_IPP
#define SYNC_DETAIL_IMPL_OUTPUT_STREAM_IPP

#include "sync/detail/output_stream.hpp"

SYNC_BEGIN
DETAIL_BEGIN


template<class OStreamType>
bool output_stream_impl<OStreamType>::good() const noexcept
{
    return _ostreamRef.get().good();
}


template<class OStreamType>
void output_stream_impl<OStreamType>::write(const char* c, std::streamsize n)
{
    _ostreamRef.get().write(c, n);
}


template<class OStreamType>
void output_stream_impl<OStreamType>::flush()
{
    _ostreamRef.get().flush();
}


// =============================================================================================


output_stream& output_stream::write(const char* c, std::streamsize n)
{
    _storage->write(c, n);
    return *this;
}


output_stream& output_stream::flush()
{
    _storage->flush();
    return *this;
}


bool output_stream::good() const noexcept
{
    return _storage->good();
}


template<class OStreamType>
void output_stream::_reset(OStreamType& ostream)
{
    using _OtherImpl = output_stream_impl<std::decay_t<OStreamType>>;
    _storage = std::make_unique<_OtherImpl>(ostream);
}


DETAIL_END
SYNC_END


#endif  // SYNC_DETAIL_IMPL_OUTPUT_STREAM_IPP