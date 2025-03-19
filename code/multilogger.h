#pragma once

#include <ostream>
#include <unordered_set>
#include <mutex>

#include "_sync_core.h"


SYNC_BEGIN

DETAIL_BEGIN

/**
 * @brief Class that manages multiple output streams in a threadsafe manner
 */
template<class CharType>
class _multilogger_impl
{
private:
    using _ostream_t = std::basic_ostream<CharType>;

    /**
     * @brief Set of stream references
     */
    std::unordered_set<_ostream_t*> _ostreams;

    /**
     * @brief Thread safety mutex for this object
     */
    mutable std::mutex _mtx;

public:

    _multilogger_impl()     = default;
    ~_multilogger_impl()    = default;

    /// @brief Move is allowed 
    _multilogger_impl(_multilogger_impl&&) noexcept             = default;
    _multilogger_impl& operator=(_multilogger_impl&&) noexcept  = default;

    /// @brief Copy is not allowed 
    _multilogger_impl(const _multilogger_impl&)             = delete;
    _multilogger_impl& operator=(const _multilogger_impl&)  = delete;

public:

    /**
     * @brief Add new reference to be managed
     * @param stream The output stream
     */
    void add(_ostream_t& stream)
    {
        std::lock_guard lock(_mtx);

        _ostreams.emplace(&stream);
    }

    /**
     * @brief Remove all stream references
     */
    void clear()
    {
        std::lock_guard lock(_mtx);

        _ostreams.clear();
    }

    /**
     * @brief Check if ostream list is empty
     * @return `true` if empty
     */
    bool empty() const
    {
        std::lock_guard lock(_mtx);

        return _ostreams.empty();
    }

    /**
     * @brief Outputs the data to each stream
     * @param data The output of the streams
     */
    template<class Type>
    _multilogger_impl& operator<<(const Type& data)
    {
        std::lock_guard lock(_mtx);

        auto currentIter    = _ostreams.begin();
        auto endIter        = _ostreams.end();

        while (currentIter != endIter)
        {
            _ostream_t* currentStreamPtr = *currentIter;

            if (currentStreamPtr == nullptr)    // The referenced object no longer exists
            {
                currentIter = _ostreams.erase(currentIter);
            }
            else
            {
                if ((*currentStreamPtr).good()) // The stream has no error flags
                {
                    (*currentStreamPtr) << data << std::flush;
                }
                else
                {
                    // do nothing - current stream has error flags
                }

                ++currentIter;
            }
        }

        return *this;
    }
};  // END _multilogger_impl

DETAIL_END

/// @brief Specialization for @c char stream
using multilogger = detail::_multilogger_impl<char>;

/// @brief Specialization for @c wchar_t stream
using wmultilogger = detail::_multilogger_impl<wchar_t>;

SYNC_END
