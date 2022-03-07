#pragma once

#include <cpp_pipelines/iter_utils.hpp>
#include <cpp_pipelines/pipeline.hpp>
#include <cpp_pipelines/subrange.hpp>

namespace cpp_pipelines::seq
{
struct drop_fn
{
    struct impl
    {
        std::ptrdiff_t n;

        template <class Range>
        constexpr auto operator()(Range&& range) const
        {
            auto b = std::begin(range);
            auto e = std::end(range);
            return subrange{ advance(b, n, e), e };
        }
    };

    constexpr auto operator()(std::ptrdiff_t n) const
    {
        return make_pipeline(impl{ n });
    }
};

static constexpr inline auto drop = drop_fn{};

}  // namespace cpp_pipelines::seq