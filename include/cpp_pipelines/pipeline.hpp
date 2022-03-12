#pragma once

#include <cpp_pipelines/invoke.hpp>
#include <tuple>

namespace cpp_pipelines
{
template <class T>
constexpr decltype(auto) to_return_type(T&& item)
{
    if constexpr (std::is_lvalue_reference_v<T>)
    {
        return item;
    }
    else
    {
        return std::decay_t<T>{ std::forward<T>(item) };
    }
}

template <class... Pipes>
struct pipeline_t
{
    std::tuple<Pipes...> pipes;

    template <class... Args>
    constexpr decltype(auto) operator()(Args&&... args) const
    {
        return to_return_type(call<0>(std::forward<Args>(args)...));
    }

private:
    template <std::size_t I, class... Args>
    constexpr decltype(auto) call(Args&&... args) const
    {
        if constexpr (I + 1 == sizeof...(Pipes))
        {
            return invoke(std::get<I>(pipes), std::forward<Args>(args)...);
        }
        else
        {
            return call<I + 1>(invoke(std::get<I>(pipes), std::forward<Args>(args)...));
        }
    }
};

struct make_pipeline_fn
{
    template <class... Pipes>
    constexpr auto operator()(Pipes... pipes) const
    {
        return from_tuple(std::tuple_cat(to_tuple(std::move(pipes))...));
    }

private:
    template <class Pipe>
    constexpr auto to_tuple(Pipe pipe) const -> std::tuple<Pipe>
    {
        return { std::move(pipe) };
    }

    template <class... Pipes>
    constexpr auto to_tuple(pipeline_t<Pipes...> pipe) const -> std::tuple<Pipes...>
    {
        return std::move(pipe.pipes);
    }

    template <class... Pipes>
    constexpr auto from_tuple(std::tuple<Pipes...> tuple) const -> pipeline_t<Pipes...>
    {
        return { std::move(tuple) };
    }
};

static constexpr inline auto make_pipeline = make_pipeline_fn{};
static constexpr inline auto fn = make_pipeline_fn{};

template <class... L, class... R>
constexpr auto operator>>=(pipeline_t<L...> lhs, pipeline_t<R...> rhs)
{
    return make_pipeline(std::move(lhs), std::move(rhs));
}

template <class T, class... Pipes>
constexpr decltype(auto) operator>>=(T&& item, const pipeline_t<Pipes...>& pipeline)
{
    return pipeline(std::forward<T>(item));
}

}  // namespace cpp_pipelines