#pragma once

#include <cpp_pipelines/pipeline.hpp>

namespace cpp_pipelines::tpl
{
namespace detail
{
struct transform_fn
{
    template <class Func>
    struct impl
    {
        Func func;

        template <class... Args>
        constexpr auto operator()(const std::tuple<Args...>& tuple) const
        {
            return call(tuple, std::index_sequence_for<Args...>{});
        }

        template <class K, class V>
        constexpr auto operator()(const std::pair<K, V>& pair) const
        {
            return std::pair{
                invoke(func, std::get<0>(pair)),
                invoke(func, std::get<1>(pair))
            };
        }

        template <class Tuple, std::size_t... I>
        constexpr auto call(const Tuple& tuple, std::index_sequence<I...>) const
        {
            return std::tuple{ invoke(func, std::get<I>(tuple))... };
        }
    };

    template <class Func>
    constexpr auto operator()(Func func) const
    {
        return make_pipeline(impl<Func>{ std::move(func) });
    }
};

struct for_each_fn
{
    template <class Func>
    struct impl
    {
        Func func;

        template <class... Args>
        constexpr void operator()(const std::tuple<Args...>& tuple) const
        {
            return call(tuple, std::index_sequence_for<Args...>{});
        }

        template <class K, class V>
        constexpr void operator()(const std::pair<K, V>& pair) const
        {
            return call(pair, std::index_sequence_for<K, V>{});
        }

        template <class Tuple, std::size_t... I>
        constexpr void call(const Tuple& tuple, std::index_sequence<I...>) const
        {
            (invoke(func, std::get<I>(tuple)), ...);
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
    template <class Func, class Args>
    struct impl
    {
        Func func;
        Args args;

        static constexpr inline std::size_t args_count = std::tuple_size_v<std::decay_t<Args>>;

        constexpr impl(Func func, Args args)
            : func{ std::move(func) }
            , args{ std::move(args) }
        {
        }

        template <class T>
        constexpr decltype(auto) operator()(T&& item) const
        {
            return call(std::forward<T>(item), std::make_index_sequence<args_count + std::tuple_size_v<std::decay_t<T>>>{});
        }

        template <class T, std::size_t... I>
        constexpr decltype(auto) call(T&& item, std::index_sequence<I...>) const
        {
            return invoke(func, get<I>(std::forward<T>(item))...);
        }

        template <std::size_t I, class T>
        constexpr decltype(auto) get(T&& item) const
        {
            if constexpr (I < args_count)
            {
                return std::get<I>(args);
            }
            else
            {
                return std::get<I - args_count>(std::forward<T>(item));
            }
        }
    };

    template <class Func, class... Args>
    constexpr auto operator()(Func func, Args... args) const
    {
        return make_pipeline(impl{ std::move(func), std::tuple{ std::move(args)... } });
    }
};

}  // namespace detail

static constexpr inline auto transform = detail::transform_fn{};
static constexpr inline auto for_each = detail::for_each_fn{};
static constexpr inline auto apply = detail::apply_fn{};
}  // namespace cpp_pipelines::tpl