#pragma once

#include <type_traits>
#include <memory>


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
    OutObject* _outObject = nullptr;

public:
    template<class OtherOutObject,
    std::enable_if_t<!std::is_same_v<std::decay_t<OtherOutObject>, output_impl>, bool> = true>
    explicit output_impl(OtherOutObject&& val)
        : _outObject(&static_cast<OtherOutObject&&>(val)) { /*Empty*/ }

    void write(const char* c, std::streamsize n) override
    {
        _outObject->write(c, n);
    }
};  // END output_impl


class output_wrapper
{
private:
    std::unique_ptr<output_interface> _storage;

public:

    output_wrapper() = default;
    ~output_wrapper() = default;

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
        // if (!detail::_test_callable(val))   // null member pointer/function pointer/custom::function
        //     return;                 // already empty

        using _OtherImpl = output_impl<std::decay_t<OutObject>>;
        _storage.reset(new _OtherImpl(val));
    }

};  // END output_wrapper
