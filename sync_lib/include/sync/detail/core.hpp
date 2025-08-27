#ifndef SYNC_DETAIL_CORE_HPP
#define SYNC_DETAIL_CORE_HPP

#include <cstddef>      // size_t, nullptr_t
#include <stdexcept>    // exceptions
#include <climits>      // numeric limits
#include <iostream>     // debugging


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

#ifndef NDEBUG
#   define _SYNC_ASSERT(Expr, Msg) __Assert(Expr, Msg, #Expr, __FILE__, __LINE__)
#else
#   define _SYNC_ASSERT(Expr, Msg) ((void)0)
#endif


#define SYNC_HEADER_ONLY 1  // may change in the future

#ifdef SYNC_HEADER_ONLY
#   define SYNC_DECL inline
#endif  // #ifdef SYNC_HEADER_ONLY

// If SYNC_DECL isn't defined yet, define it now.
#ifndef SYNC_DECL
# define SYNC_DECL
#endif // !defined(SYNC_DECL)

#define DETAIL_BEGIN namespace detail {
#define DETAIL_END }

#define SYNC_BEGIN namespace sync {
#define SYNC_END }

#endif  // SYNC_DETAIL_CORE_HPP