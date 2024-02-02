#pragma once

#include <cpp_pipelines/iter_utils.hpp>
#include <cpp_pipelines/pipeline.hpp>
#include <cpp_pipelines/subrange.hpp>

namespace cpp_pipelines::sub
{
namespace detail
{
struct take_fn
{
    struct impl
    {
        std::ptrdiff_t count;

        template <class Range>
        constexpr auto operator()(Range&& range) const
        {
            const auto b = std::begin(range);
            const auto e = std::end(range);

            return subrange{ b, advance(b, count, e) };
        }
    };

    constexpr auto operator()(std::ptrdiff_t count) const
    {
        return fn(impl{ count });
    }
};
}  // namespace detail

static constexpr inline auto take = detail::take_fn{};
}  // namespace cpp_pipelines::sub