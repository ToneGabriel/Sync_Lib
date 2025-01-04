#pragma once

#include <ostream>
#include <unordered_set>
#include <mutex>

#include "_Core.h"


SYNC_BEGIN


DETAIL_BEGIN


template<class CharType>
class basic_multilogger
{
private:
    using _ostream_t = std::basic_ostream<CharType>;

    std::unordered_set<_ostream_t*> _ostreams;
    std::mutex                      _mtx;

public:

    basic_multilogger()     = default;
    ~basic_multilogger()    = default;

    basic_multilogger(basic_multilogger&&)              = default;
    basic_multilogger& operator=(basic_multilogger&&)   = default;

    basic_multilogger(const basic_multilogger&)             = delete;
    basic_multilogger& operator=(const basic_multilogger&)  = delete;

public:

    void add(_ostream_t& stream)
    {
        std::lock_guard lock(_mtx);

        _ostreams.emplace(&stream);
    }

    void clear()
    {
        std::lock_guard lock(_mtx);

        _ostreams.clear();
    }

    template<class Type>
    basic_multilogger& operator<<(const Type& data)
    {
        std::lock_guard lock(_mtx);

        auto currentIter    = _ostreams.begin();
        auto endIter        = _ostreams.end();

        while (currentIter != endIter)
        {
            _ostream_t* currentStreamPtr = *currentIter;

            if (currentStreamPtr == nullptr)
                currentIter = _ostreams.erase(currentIter);
            else
            {
                if (currentStreamPtr->good())
                    *currentStreamPtr << data << std::flush;
                else
                {
                    // do nothing - current stream has error flags
                }

                ++currentIter;
            }
        }

        return *this;
    }
};  // END basic_multilogger


DETAIL_END


using multilogger   = detail::basic_multilogger<char>;
using wmultilogger  = detail::basic_multilogger<wchar_t>;


SYNC_END
