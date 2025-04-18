#pragma once

#include <cstddef>      // size_t, nullptr_t
#include <stdexcept>    // exceptions
#include <climits>      // numeric limits
#include <iostream>     // debugging

#include "_sync_config.hpp"   // project configuration


inline void __Assert(bool expr, const char* msg, const char* expected, const char* file, int line)
{
    if (!expr)
    {
        std::cerr   << "Assert failed:\t"   << msg      << "\n"
                    << "Expected:\t"        << expected << "\n"
                    << "File:\t\t"          << file     << "\n"
                    << "Line:\t\t"          << line     << "\n";
        ::abort();
    }
    else
    {
        // assert OK - do nothing
    }
}

#define _ASSERT(Expr, Msg) __Assert(Expr, Msg, #Expr, __FILE__, __LINE__)

#define RERAISE throw    // used to terminate in a catch block

#define DETAIL_BEGIN namespace detail {
#define DETAIL_END }

#define SYNC_BEGIN namespace sync {
#define SYNC_END }
