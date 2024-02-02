#pragma once

#include <cpp_pipelines/invoke.hpp>
#include <cpp_pipelines/pipeline.hpp>

namespace cpp_pipelines::seq
{
namespace detail
{
struct for_each_fn
{
    template <class Func>
    struct impl
    {
        Func func;

        template <class Range>
        constexpr auto operator()(Range&& range) const
        {
            for (auto&& item : std::forward<Range>(range))
            {
                invoke(func, std::forward<decltype(item)>(item));
            }
            return func;
        }
    };

    template <class Func>
    constexpr auto operator()(Func func) const
    {
        return fn(impl<Func>{ std::move(func) });
    }
};

}  // namespace detail

static constexpr inline auto for_each = detail::for_each_fn{};

}  // namespace cpp_pipelines::seq