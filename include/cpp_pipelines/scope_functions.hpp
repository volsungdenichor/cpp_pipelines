#pragma once

#include <cpp_pipelines/pipeline.hpp>

namespace cpp_pipelines
{
namespace detail
{
struct inspect_fn
{
    template <class Func>
    struct impl
    {
        Func func;

        template <class T>
        constexpr decltype(auto) operator()(T&& item) const
        {
            invoke(func, item);
            return to_return_type(std::forward<T>(item));
        }
    };

    template <class Func>
    constexpr auto operator()(Func func) const
    {
        return make_pipeline(impl<Func>{ std::move(func) });
    }
};

struct apply_fn
{
    template <class Func>
    struct impl
    {
        Func func;

        template <class T>
        constexpr T& operator()(T& item) const
        {
            invoke(func, item);
            return item;
        }
    };

    template <class Func>
    constexpr auto operator()(Func func) const
    {
        return make_pipeline(impl<Func>{ std::move(func) });
    }
};

struct with_fn
{
    template <class Func>
    struct impl
    {
        Func func;

        template <class T>
        constexpr T operator()(T item) const
        {
            invoke(func, item);
            return item;
        }
    };

    template <class Func>
    constexpr auto operator()(Func func) const
    {
        return make_pipeline(impl<Func>{ std::move(func) });
    }
};

}  // namespace detail

static constexpr inline auto inspect = detail::inspect_fn{};
static constexpr inline auto tap = inspect;

static constexpr inline auto apply = detail::apply_fn{};
static constexpr inline auto with = detail::with_fn{};

template <class T>
constexpr T create(const std::function<void(T&)>& func)
{
    T result{};
    func(result);
    return result;
}

}  // namespace cpp_pipelines