#pragma once

#include <cpp_pipelines/iter_utils.hpp>
#include <cpp_pipelines/pipeline.hpp>
#include <cpp_pipelines/subrange.hpp>

namespace cpp_pipelines::seq
{
struct take_fn
{
    struct impl
    {
        std::ptrdiff_t n;

        template <class Range>
        constexpr auto operator()(Range&& range) const
        {
            auto b = std::begin(range);
            auto e = std::end(range);
            return subrange{ b, advance(b, n, e) };
        }
    };

    constexpr auto operator()(std::ptrdiff_t n) const
    {
        return make_pipeline(impl{ n });
    }
};  // namespace cpp_pipelines::seq

static constexpr inline auto take = take_fn{};
}  // namespace cpp_pipelines::seq