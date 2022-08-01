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
        constexpr T operator()(T&& item) const
        {
            invoke(func, item);
            return std::forward<T>(item);
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

struct do_all_fn
{
    template <class... Funcs>
    struct impl
    {
        std::tuple<Funcs...> funcs;

        template <class... Args>
        constexpr void operator()(Args&&... args) const
        {
            call(std::index_sequence_for<Funcs...>{}, std::forward<Args>(args)...);
        }

        template <class... Args, std::size_t... I>
        constexpr void call(std::index_sequence<I...>, Args&&... args) const
        {
            (invoke(std::get<I>(funcs), std::forward<Args>(args)...), ...);
        }
    };

    template <class... Funcs>
    constexpr auto operator()(Funcs... funcs) const
    {
        return make_pipeline(impl<Funcs...>{ std::tuple{ std::move(funcs)... } });
    }
};

}  // namespace detail

static constexpr inline auto inspect = detail::inspect_fn{};
static constexpr inline auto tap = inspect;

static constexpr inline auto apply = detail::apply_fn{};
static constexpr inline auto with = detail::with_fn{};

static constexpr inline auto do_all = detail::do_all_fn{};

template <class T>
constexpr T create(const std::function<void(T&)>& func)
{
    T result{};
    func(result);
    return result;
}

}  // namespace cpp_pipelines