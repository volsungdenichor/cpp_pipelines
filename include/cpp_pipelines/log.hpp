#pragma once

#include <cpp_pipelines/output.hpp>
#include <cpp_pipelines/pipeline.hpp>
#include <iterator>
#include <string>
#include <vector>

namespace cpp_pipelines
{
namespace log
{
struct logs_t : std::vector<std::string>
{
    using log_type = std::string;
    using base_type = std::vector<std::string>;
    using base_type::base_type;

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

struct value_fn
{
    template <class T>
    constexpr auto operator()(T&& item) const
    {
        for (const auto& log : item.logs)
        {
            std::cout << "| " << log << std::endl;
        }
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
static constexpr inline auto value = make_pipeline(detail::value_fn{});
static constexpr inline auto invoke = make_pipeline(detail::invoke_fn{});

}  // namespace log

}  // namespace cpp_pipelines