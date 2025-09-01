#ifndef SYNC_MULTILOGGER_HPP
#define SYNC_MULTILOGGER_HPP

#include <mutex>
#include <vector>

#include "sync/detail/output_stream.hpp"


SYNC_BEGIN


class multilogger
{
private:
    std::vector<detail::output_stream> _ostreams;
    mutable std::mutex _mtx;

public:

    multilogger()     = default;
    ~multilogger()    = default;

    /// @brief Move is allowed 
    multilogger(multilogger&&) noexcept             = default;
    multilogger& operator=(multilogger&&) noexcept  = default;

    /// @brief Copy is not allowed 
    multilogger(const multilogger&)             = delete;
    multilogger& operator=(const multilogger&)  = delete;

public:

    template<class StreamType>
    void add(StreamType& ostream);

    void clear();
    bool empty() const;
    void write(const char* c, std::streamsize n);
};  // END multilogger


SYNC_END

#include "sync/detail/impl/multilogger.ipp"

#endif  // SYNC_MULTILOGGER_HPP
