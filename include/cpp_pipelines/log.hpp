#pragma once

#include <algorithm>
#include <cpp_pipelines/output.hpp>
#include <cpp_pipelines/pipeline.hpp>
#include <iterator>
#include <string>
#include <vector>

namespace cpp_pipelines
{
namespace log
{
enum category
{
    info,
    debug,
    warning,
    error
};

struct log_t
{
    std::string message;
    category cat;

    log_t(std::string message, category cat = category::debug)
        : message{ std::move(message) }
        , cat{ cat }
    {
    }

    friend std::ostream& operator<<(std::ostream& os, const log_t& item)
    {
        return os << item.message;
    }
};
struct logs_t : std::vector<log_t>
{
    using log_type = log_t;
    using base_type = std::vector<log_t>;
    using base_type::base_type;

    logs_t(std::initializer_list<std::string> init)
    {
        std::transform(std::begin(init), std::end(init), std::back_inserter(*this), [](std::string item) { return log_t{ std::move(item) }; });
    }

    logs_t& operator+=(logs_t other)
    {
        (*this).insert(std::end(*this), std::move_iterator{ std::begin(other) }, std::move_iterator{ std::end(other) });
        return *this;
    }

    logs_t& operator+=(log_type log)
    {
        (*this).push_back(std::move(log));
        return *this;
    }

    logs_t operator+(logs_t other) const
    {
        return logs_t{ *this } += std::move(other);
    }

    logs_t operator+(log_type log) const
    {
        return logs_t{ *this } += std::move(log);
    }
};

template <class T>
struct value_wrapper
{
    T value;
    logs_t logs;

    value_wrapper(T value, logs_t logs = {})
        : value{ std::move(value) }
        , logs{ std::move(logs) }
    {
    }

    constexpr const T& operator*() const&
    {
        return value;
    }

    constexpr T&& operator*() &&
    {
        return std::move(value);
    }

    friend std::ostream& operator<<(std::ostream& os, const value_wrapper& item)
    {
        return os << item.value;
    }
};

namespace detail
{
struct lift_fn
{
    template <class T>
    constexpr auto operator()(T&& value, logs_t logs = {}) const
    {
        return value_wrapper<std::decay_t<T>>{ std::forward<T>(value), std::move(logs) };
    }
};

static constexpr inline auto lift = make_pipeline(lift_fn{});

struct and_then_fn
{
    template <class Func>
    struct impl
    {
        Func func;

        template <class T>
        constexpr auto operator()(T&& item) const
        {
            auto res = invoke(func, *std::forward<T>(item));
            return lift(
                *std::move(res),
                item.logs + std::move(res.logs));
        }
    };

    template <class Func>
    constexpr auto operator()(Func func) const
    {
        return make_pipeline(impl<Func>{ std::move(func) });
    }
};

struct transform_fn
{
    template <class Func>
    struct impl
    {
        Func func;

        template <class T>
        constexpr auto operator()(T&& item) const
        {
            return lift(invoke(func, *std::forward<T>(item)));
        }
    };

    template <class Func>
    constexpr auto operator()(Func func) const
    {
        return make_pipeline(impl<Func>{ std::move(func) });
    }
};

struct flush_fn
{
    template <class Handler>
    struct impl
    {
        Handler handler;

        template <class T>
        constexpr auto operator()(T&& item) const
        {
            std::for_each(std::begin(item.logs), std::end(item.logs), std::ref(handler));
            return lift(*std::forward<T>(item));
        }
    };

    template <class Handler>
    constexpr auto operator()(Handler handler) const
    {
        return make_pipeline(impl<Handler>{ std::move(handler) });
    }
};

struct value_fn
{
    template <class T>
    constexpr decltype(auto) operator()(T&& item) const
    {
        return *std::forward<T>(item);
    }
};

struct append_logs_fn
{
    struct impl
    {
        logs_t logs;

        template <class T>
        constexpr auto operator()(T&& item) const
        {
            return lift(*std::forward<T>(item), item.logs + logs);
        }
    };

    auto operator()(logs_t logs) const
    {
        return make_pipeline(impl{ std::move(logs) });
    }
};

struct invoke_fn
{
    template <class Func>
    struct impl
    {
        Func func;
        std::string msg;

        template <class... Args>
        auto operator()(Args&&... args) const
        {
            auto in = str(msg, " in: ", std::tie(args...));
            auto res = invoke(func, std::forward<Args>(args)...);
            auto out = str(msg, " out: ", *res);
            return lift(
                *std::move(res),
                logs_t{ std::move(in) } + res.logs + logs_t{ std::move(out) });
        }
    };
    template <class Func>
    constexpr auto operator()(Func func, std::string msg = {}) const
    {
        return make_pipeline(impl<Func>{ std::move(func), std::move(msg) });
    }
};

}  // namespace detail

using detail::lift;

static constexpr inline auto and_then = detail::and_then_fn{};
static constexpr inline auto transform = detail::transform_fn{};
static constexpr inline auto append_logs = detail::append_logs_fn{};
static constexpr inline auto flush = detail::flush_fn{};
static constexpr inline auto value = make_pipeline(detail::value_fn{});

}  // namespace log

}  // namespace cpp_pipelines