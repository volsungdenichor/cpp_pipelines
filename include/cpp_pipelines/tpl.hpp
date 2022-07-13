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
    template <class Func>
    struct impl
    {
        Func func;

        template <class T>
        constexpr decltype(auto) operator()(T&& item) const
        {
            return std::apply(func, std::forward<T>(item));
        }
    };

    template <class Func>
    constexpr auto operator()(Func func) const
    {
        return make_pipeline(impl<Func>{ std::move(func) });
    }
};

}  // namespace detail

static constexpr inline auto transform = detail::transform_fn{};
static constexpr inline auto for_each = detail::for_each_fn{};
static constexpr inline auto apply = detail::apply_fn{};
}  // namespace cpp_pipelines::tpl