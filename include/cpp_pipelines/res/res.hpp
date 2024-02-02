#pragma once

#include <cpp_pipelines/opt.hpp>
#include <cpp_pipelines/output.hpp>
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

template <class T>
class bad_result_access : public std::runtime_error
{
public:
    bad_result_access(T error)
        : std::runtime_error{ str(error) }
    {
    }
};

template <>
class bad_result_access<void> : public std::runtime_error
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

    constexpr result(const result&) = default;
    constexpr result(result&&) = default;

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
            throw bad_result_access<error_type>{ std::get<error_data>(data).error };
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

template <class... L, class... R>
constexpr bool operator==(const result<L...>& lhs, const result<R...>& rhs)
{
    if (lhs.has_value() && rhs.has_value())
    {
        return lhs.value() == rhs.value();
    }
    else if (lhs.has_error() && rhs.has_error())
    {
        return lhs.error() == rhs.error();
    }
    else
    {
        return false;
    }
}

template <class... L, class... R>
constexpr bool operator!=(const result<L...>& lhs, const result<R...>& rhs)
{
    return !(lhs == rhs);
}

template <class... L, class R>
constexpr bool operator==(const result<L...>& lhs, const error_wrapper<R>& rhs)
{
    return lhs.has_error() && lhs.error() == rhs.error;
}

template <class... L, class R>
constexpr bool operator!=(const result<L...>& lhs, const error_wrapper<R>& rhs)
{
    return !(lhs == rhs);
}

template <class L, class... R>
constexpr bool operator==(const error_wrapper<L>& lhs, const result<R...>& rhs)
{
    return rhs == lhs;
}

template <class L, class... R>
constexpr bool operator!=(const error_wrapper<L>& lhs, const result<R...>& rhs)
{
    return !(lhs == rhs);
}

template <class... L, class R>
constexpr bool operator==(const result<L...>& lhs, const R& rhs)
{
    return lhs.has_value() && lhs.value() == rhs;
}

template <class... L, class R>
constexpr bool operator!=(const result<L...>& lhs, const R& rhs)
{
    return !(lhs == rhs);
}

template <class L, class... R>
constexpr bool operator==(const L& lhs, const result<R...>& rhs)
{
    return rhs == lhs;
}

template <class L, class... R>
constexpr bool operator!=(const L& lhs, const result<R...>& rhs)
{
    return !(lhs == rhs);
}

template <class T, class E>
std::ostream& operator<<(std::ostream& os, const result<T, E>& item)
{
    if (item)
        return os << "ok{" << item.value() << "}";
    else
        return os << "error{" << item.error() << "}";
}
namespace res
{
namespace detail
{
template <class Res>
constexpr bool has_value(Res&& item)
{
    return static_cast<bool>(item);
}

template <class Res>
constexpr decltype(auto) get_value(Res&& item)
{
    return to_return_type(*std::forward<Res>(item));
}

template <class Res>
constexpr decltype(auto) get_error(Res&& item)
{
    return to_return_type(std::forward<Res>(item).error());
}

struct and_then_fn
{
    template <class Func>
    struct impl
    {
        Func func;

        template <class Res>
        constexpr auto operator()(Res&& res) const
        {
            using result_type = std::decay_t<decltype(invoke(func, get_value(std::forward<Res>(res))))>;
            return has_value(res)
                       ? result_type{ invoke(func, get_value(std::forward<Res>(res))) }
                       : result_type{ error(get_error(std::forward<Res>(res))) };
        }
    };

    template <class Func>
    constexpr auto operator()(Func func) const
    {
        return fn(impl<Func>{ std::move(func) });
    }
};
struct transform_fn
{
    template <class Func>
    struct impl
    {
        Func func;

        template <class Res>
        constexpr auto operator()(Res&& res) const
        {
            using T = std::decay_t<decltype(invoke(func, get_value(std::forward<Res>(res))))>;
            using E = std::decay_t<decltype(get_error(std::forward<Res>(res)))>;
            using result_type = result<T, E>;
            return has_value(res)
                       ? result_type{ invoke(func, get_value(std::forward<Res>(res))) }
                       : result_type{ error(get_error(std::forward<Res>(res))) };
        }
    };

    template <class Func>
    constexpr auto operator()(Func func) const
    {
        return fn(impl<Func>{ std::move(func) });
    }
};

struct transform_error_fn
{
    template <class Func>
    struct impl
    {
        Func func;

        template <class Res>
        constexpr auto operator()(Res&& res) const
        {
            using T = std::decay_t<decltype(get_value(std::forward<Res>(res)))>;
            using E = std::decay_t<decltype(invoke(func, get_error(std::forward<Res>(res))))>;
            using result_type = result<T, E>;
            return res
                       ? result_type{ get_value(std::forward<Res>(res)) }
                       : result_type{ error(invoke(func, get_error(std::forward<Res>(res)))) };
        }
    };

    template <class Func>
    constexpr auto operator()(Func func) const
    {
        return fn(impl<Func>{ std::move(func) });
    }
};

struct value_fn
{
    template <class Res>
    constexpr decltype(auto) operator()(Res&& res) const
    {
        return to_return_type(std::forward<Res>(res).value());
    }
};

struct error_fn
{
    template <class Res>
    constexpr decltype(auto) operator()(Res&& res) const
    {
        return get_error(std::forward<Res>(res));
    }
};

struct maybe_value_fn
{
    template <class Res>
    constexpr auto operator()(Res&& res) const
    {
        using result_type = decltype(opt::lift(get_value(std::forward<Res>(res))));
        return res
                   ? opt::lift(get_value(std::forward<Res>(res)))
                   : result_type{};
    }
};

struct maybe_error_fn
{
    template <class Res>
    constexpr auto operator()(Res&& res) const
    {
        using result_type = decltype(opt::lift(get_error(std::forward<Res>(res))));
        return !res
                   ? opt::lift(get_error(std::forward<Res>(res)))
                   : result_type{};
    }
};

struct match_fn
{
    template <class OnValue, class OnError>
    struct impl
    {
        OnValue on_value;
        OnError on_error;

        template <class Res>
        constexpr decltype(auto) operator()(Res&& res) const
        {
            return has_value(res)
                       ? to_return_type(invoke(on_value, get_value(std::forward<Res>(res))))
                       : to_return_type(invoke(on_error, get_error(std::forward<Res>(res))));
        }
    };

    template <class OnValue, class OnError>
    constexpr auto operator()(OnValue on_value, OnError on_error) const
    {
        return fn(impl<OnValue, OnError>{ std::move(on_value), std::move(on_error) });
    }
};

struct try_invoke_fn
{
    template <class Func, class... Args>
    auto operator()(Func&& func, Args&&... args) const
    {
        using res = decltype(invoke(std::forward<Func>(func), std::forward<Args>(args)...));
        using result_type = result<res, std::string>;

        try
        {
            return result_type{ invoke(std::forward<Func>(func), std::forward<Args>(args)...) };
        }
        catch (const std::exception& ex)
        {
            return result_type{ error(std::string{ ex.what() }) };
        }
    }
};

}  // namespace detail

static constexpr inline auto value = fn(detail::value_fn{});
using opt::value_or;
using opt::value_or_else;
using opt::value_or_throw;

static constexpr inline auto and_then = detail::and_then_fn{};
using opt::inspect;
using opt::or_else;

using opt::all_of;
using opt::any_of;
using opt::matches;
using opt::none_of;

using opt::accumulate;

static constexpr inline auto transform = detail::transform_fn{};
static constexpr inline auto transform_error = detail::transform_error_fn{};

static constexpr inline auto error = fn(detail::error_fn{});
static constexpr inline auto maybe_value = fn(detail::maybe_value_fn{});
static constexpr inline auto maybe_error = fn(detail::maybe_error_fn{});

static constexpr inline auto match = detail::match_fn{};

static constexpr inline auto try_invoke = detail::try_invoke_fn{};

}  // namespace res

}  // namespace cpp_pipelines