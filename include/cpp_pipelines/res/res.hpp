#pragma once

#include <cpp_pipelines/opt.hpp>
#include <variant>

namespace cpp_pipelines
{
template <class E>
struct error_wrapper
{
    E error;
};

template <class E>
constexpr auto error(E&& e)
{
    return error_wrapper<std::decay_t<E>>{ std::forward<E>(e) };
}

class bad_result_access : public std::runtime_error
{
public:
    bad_result_access(std::string msg)
        : std::runtime_error{ std::move(msg) }
    {
    }
};

template <class T, class E>
class result
{
public:
    using value_type = T;
    using error_type = E;

    using value_data = value_type;
    using error_data = error_wrapper<error_type>;

    constexpr result()
        : data{ value_data{} }
    {
    }

    template <class U, class = std::enable_if_t<std::is_constructible_v<value_type, U>>>
    constexpr result(U&& value)
        : data{ value_data{ std::forward<U>(value) } }
    {
    }

    template <class OE>
    constexpr result(error_wrapper<OE> error)
        : data{ error_data{ std::move(error.error) } }
    {
    }

    template <class OT, class OE>
    constexpr result(result<OT, OE> other)
        : data{ std::move(other.data) }
    {
    }

    constexpr result& operator=(result other)
    {
        std::swap(data, other.data);
        return *this;
    }

    constexpr explicit operator bool() const
    {
        return std::holds_alternative<value_data>(data);
    }

    constexpr const value_type& operator*() const&
    {
        return std::get<value_data>(data);
    }

    constexpr value_type& operator*() &
    {
        return std::get<value_data>(data);
    }

    constexpr value_type&& operator*() &&
    {
        return std::get<value_data>(std::move(data));
    }

    constexpr const value_type* operator->() const&
    {
        return std::addressof(**this);
    }

    constexpr value_type* operator->() &
    {
        return std::addressof(**this);
    }

    constexpr bool has_value() const
    {
        return !!(*this);
    }

    constexpr bool has_error() const
    {
        return !(*this);
    }

    constexpr const value_type& value() const&
    {
        ensure_has_value();
        return **this;
    }

    constexpr value_type& value() &
    {
        ensure_has_value();
        return **this;
    }

    constexpr value_type&& value() &&
    {
        ensure_has_value();
        return *std::move(*this);
    }

    constexpr const error_type& error() const&
    {
        ensure_has_error();
        return std::get<error_data>(data).error;
    }

    constexpr error_type& error() &
    {
        ensure_has_error();
        return std::get<error_data>(data).error;
    }

    constexpr error_type&& error() &&
    {
        ensure_has_error();
        return std::get<error_data>(std::move(data)).error;
    }

private:
    constexpr void ensure_has_value() const
    {
        if (!has_value())
            throw bad_result_access{ "value expected, got error" };
    }

    constexpr void ensure_has_error() const
    {
        if (!has_error())
            throw bad_result_access{ "error expected, got value" };
    }
    std::variant<value_data, error_data> data;
};

template <class T, class E>
struct opt::customization::optional_traits<result<T, E>>
{
    using type = result<T, E>;
    constexpr bool has_value(const type& item) const
    {
        return static_cast<bool>(item);
    }

    constexpr const T& get_value(const type& item) const
    {
        return *item;
    }

    constexpr T& get_value(type& item) const
    {
        return *item;
    }

    constexpr T&& get_value(type&& item) const
    {
        return *std::move(item);
    }
};

template <class T, class E>
struct opt::customization::optional_traits<result<std::reference_wrapper<T>, E>>
{
    using type = result<std::reference_wrapper<T>, E>;

    constexpr bool has_value(const type& item) const
    {
        return static_cast<bool>(item);
    }

    constexpr T& get_value(const type& item) const
    {
        return *item;
    }
};

template <class T, class E>
std::ostream& operator<<(std::ostream& os, const result<T, E>& item)
{
    if (item)
        return os << "ok(" << item.value() << ")";
    else
        return os << "error(" << item.error() << ")";
}

namespace res
{
using opt::filter;
using opt::value;
using opt::value_or;
using opt::value_or_else;
using opt::value_or_throw;

using opt::and_then;
using opt::inspect;

using opt::all_of;
using opt::any_of;
using opt::matches;
using opt::none_of;

using opt::accumulate;

}  // namespace res

}  // namespace cpp_pipelines