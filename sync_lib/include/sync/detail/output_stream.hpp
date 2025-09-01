#ifndef SYNC_DETAIL_OUTPUT_STREAM_HPP
#define SYNC_DETAIL_OUTPUT_STREAM_HPP

#include <type_traits>
#include <memory>

#include "sync/detail/core.hpp"

SYNC_BEGIN
DETAIL_BEGIN


class output_stream_interface
{
public:
    output_stream_interface() = default;
    virtual ~output_stream_interface() = default;

    output_stream_interface(const output_stream_interface&) = delete;
    output_stream_interface& operator=(const output_stream_interface&) = delete;

    virtual bool good() const noexcept = 0;
    virtual void flush() = 0;
    virtual void write(const char* c, std::streamsize n) = 0;
};  // END output_stream_interface


// =============================================================================================


template<class OStreamType>
class output_stream_impl final : public output_stream_interface
{
private:
    std::reference_wrapper<OStreamType> _ostreamRef;

public:
    template<class OtherOStreamType, std::enable_if_t<!std::is_same_v<std::decay_t<OtherOStreamType>, output_stream_impl>, bool> = true>
    explicit output_stream_impl(OtherOStreamType& ostream) noexcept
        : _ostreamRef(ostream) { /*Empty*/ }

    bool good() const noexcept override;
    void flush() override;
    void write(const char* c, std::streamsize n) override;
};  // END output_stream_impl


// =============================================================================================


/**
 * @brief Exception thrown when output_stream functions are called with empty storage
 */
class bad_output_stream : public std::exception
{
public:
    bad_output_stream() noexcept { /*Empty*/ }

    const char* what() const noexcept override { return "Bad output stream call."; }
};  // END bad_output_stream


class output_stream
{
private:
    std::unique_ptr<output_stream_interface> _storage;

public:

    output_stream() = default;
    ~output_stream() = default;

    // disable copy
    output_stream(const output_stream&) = delete;
    output_stream& operator=(const output_stream&) = delete;

    // allow move
    output_stream(output_stream&&) noexcept = default;
    output_stream& operator=(output_stream&&) noexcept = default;

    template<class OStreamType, std::enable_if_t<!std::is_same_v<std::decay_t<OStreamType>, output_stream>, bool> = true>
    output_stream(OStreamType&& ostream) : _storage(nullptr)
    {
        _reset(ostream);
    }

public:

    bool good() const;
    output_stream& flush();
    output_stream& write(const char* c, std::streamsize n);

private:
    template<class OStreamType>
    void _reset(OStreamType&& ostream);
};  // END output_stream


DETAIL_END
SYNC_END

#include "sync/detail/impl/output_stream.ipp"

#endif  // SYNC_DETAIL_OUTPUT_STREAM_HPP
