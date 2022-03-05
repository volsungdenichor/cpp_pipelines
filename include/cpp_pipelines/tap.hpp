#pragma once

#include <cpp_pipelines/pipeline.hpp>

namespace cpp_pipelines
{
struct tap_fn
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

static constexpr inline auto tap = tap_fn{};

}  // namespace cpp_pipelines