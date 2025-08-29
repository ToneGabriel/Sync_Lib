#ifndef SYNC_MULTILOGGER_HPP
#define SYNC_MULTILOGGER_HPP

#include <mutex>
#include <vector>

#include "sync/detail/output_interface.hpp"


SYNC_BEGIN

class multilogger
{
private:
    std::vector<output_wrapper> _loggers;
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

    template<class OutObject>
    void add(OutObject& obj)
    {
        std::lock_guard lock(_mtx);
        _loggers.push_back(output_wrapper(obj));
    }

    void clear()
    {
        std::lock_guard lock(_mtx);
        _loggers.clear();
    }

    bool empty() const
    {
        std::lock_guard lock(_mtx);
        return _loggers.empty();
    }

    void write(const char* c, std::streamsize n)
    {
        std::lock_guard lock(_mtx);

        for (auto& lg : _loggers)
        {
            try
            {
                lg.write(c, n);
            }
            catch(...)
            {
                // TODO
            }       
        }
    }
};  // END multilogger

SYNC_END

#endif  // SYNC_MULTILOGGER_HPP
