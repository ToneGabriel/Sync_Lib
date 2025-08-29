#pragma once

#include <type_traits>
#include <memory>

#include "sync/detail/core.hpp"

SYNC_BEGIN

class output_interface
{
public:
    output_interface() = default;
    output_interface(const output_interface&) = delete;
    output_interface& operator=(const output_interface&) = delete;

    virtual ~output_interface() = default;
    virtual void write(const char* c, std::streamsize n) = 0;
};  // END output_interface


template<class OutObject>
class output_impl final : public output_interface
{
private:
    std::reference_wrapper<OutObject> _outObjectRef;

public:
    template<class OtherOutObject, std::enable_if_t<!std::is_same_v<std::decay_t<OtherOutObject>, output_impl>, bool> = true>
    explicit output_impl(OtherOutObject& val) noexcept
        : _outObjectRef(val) { /*Empty*/ }

    void write(const char* c, std::streamsize n) override
    {
        _outObjectRef.get().write(c, n);
    }
};  // END output_impl


class output_wrapper
{
private:
    std::unique_ptr<output_interface> _storage;

public:

    output_wrapper() = default;
    ~output_wrapper() = default;

    // disable copy
    output_wrapper(const output_wrapper&) = delete;
    output_wrapper& operator=(const output_wrapper&) = delete;

    // allow move
    output_wrapper(output_wrapper&&) noexcept = default;
    output_wrapper& operator=(output_wrapper&&) noexcept = default;

    template<class OutObject, std::enable_if_t<!std::is_same_v<std::decay_t<OutObject>, output_wrapper>, bool> = true>
    output_wrapper(OutObject& val) : _storage(nullptr)
    {
        _reset(val);
    }

public:

    void write(const char* c, std::streamsize n)
    {
        _storage->write(c, n);
    }

private:
    // Helpers

    template<class OutObject>
    void _reset(OutObject& val)
    {
        using _OtherImpl = output_impl<std::decay_t<OutObject>>;
        _storage = std::make_unique<_OtherImpl>(val);
    }
};  // END output_wrapper

SYNC_END
