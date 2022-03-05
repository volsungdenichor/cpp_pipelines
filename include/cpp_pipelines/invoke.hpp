#pragma once

#include <cpp_pipelines/debug.hpp>
#include <cpp_pipelines/type_traits.hpp>
#include <functional>

namespace cpp_pipelines
{
template <class T, class = std::void_t<>>
struct is_tuple_like : std::false_type
{
};

template <class T>
struct is_tuple_like<T, std::void_t<decltype(std::get<0>(std::declval<T>()))>> : std::true_type
{
};

struct invoke_fn
{
    template <class Func, class... Args>
    constexpr decltype(auto) operator()(Func&& func, Args&&... args) const
    {
        return std::invoke(std::forward<Func>(func), unwrap(std::forward<Args>(args)...));
    }

    template <class Func, class Arg>
    constexpr decltype(auto) operator()(Func&& func, Arg&& arg) const
    {
        if constexpr (std::is_invocable_v<Func, decltype(unwrap(std::forward<Arg>(arg)))>)
        {
            return std::invoke(std::forward<Func>(func), unwrap(std::forward<Arg>(arg)));
        }
        else if constexpr (is_tuple_like<std::decay_t<Arg>>::value)
        {
            return call(std::forward<Func>(func), std::forward<Arg>(arg), std::make_index_sequence<std::tuple_size_v<std::decay_t<Arg>>>{});
        }
    }

    template <class Func, class Arg, std::size_t... I>
    constexpr decltype(auto) call(Func&& func, Arg&& arg, std::index_sequence<I...>) const
    {
        return (*this)(std::forward<Func>(func), std::get<I>(std::forward<Arg>(arg))...);
    }

    template <class T>
    static constexpr T&& unwrap(T&& item)
    {
        return std::forward<T>(item);
    }

    template <class T>
    static constexpr T& unwrap(std::reference_wrapper<T> item)
    {
        return item;
    }
};

static constexpr inline auto invoke = invoke_fn{};

}  // namespace cpp_pipelines