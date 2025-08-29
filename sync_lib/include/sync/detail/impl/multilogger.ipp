#ifndef SYNC_DETAIL_IMPL_MULTILOGGER_IPP
#define SYNC_DETAIL_IMPL_MULTILOGGER_IPP

#include "sync/multilogger.hpp"

SYNC_BEGIN


template<class StreamType>
void multilogger::add(StreamType& ostream)
{
    std::lock_guard lock(_mtx);
    _ostreams.push_back(detail::output_stream(ostream));
}

void multilogger::clear()
{
    std::lock_guard lock(_mtx);
    _ostreams.clear();
}

bool multilogger::empty() const
{
    std::lock_guard lock(_mtx);
    return _ostreams.empty();
}

void multilogger::write(const char* c, std::streamsize n)
{
    std::lock_guard lock(_mtx);

    for (auto& lg : _ostreams)
    {
        try
        {
            if (lg.good())
                lg.write(c, n).flush();
        }
        catch(...)
        {
            // TODO
        }       
    }
}


SYNC_END

#endif  // SYNC_DETAIL_IMPL_MULTILOGGER_IPP