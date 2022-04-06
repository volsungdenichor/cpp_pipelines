#pragma once

#include <algorithm>
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
            return std::for_each(std::begin(range), std::end(range), func);
        }
    };

    template <class Func>
    constexpr auto operator()(Func func) const
    {
        return make_pipeline(impl<Func>{ std::move(func) });
    }
};

}  // namespace detail

static constexpr inline auto for_each = detail::for_each_fn{};

}  // namespace cpp_pipelines::seq