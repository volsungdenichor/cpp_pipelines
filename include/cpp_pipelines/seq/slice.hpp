#pragma once

#include <cpp_pipelines/iter_utils.hpp>
#include <cpp_pipelines/pipeline.hpp>
#include <cpp_pipelines/subrange.hpp>

namespace cpp_pipelines::seq
{
struct slice_fn
{
    struct impl
    {
        std::ptrdiff_t offset;
        std::ptrdiff_t count;

        template <class Range>
        constexpr auto operator()(Range&& range) const
        {
            auto b = std::begin(range);
            auto e = std::end(range);
            auto new_b = advance(b, offset, e);
            auto new_e = advance(new_b, count, e);
            return subrange{ new_b, new_e };
        }
    };

    constexpr auto operator()(std::ptrdiff_t offset, std::ptrdiff_t count) const
    {
        return make_pipeline(impl{ offset, count });
    }
};  // namespace cpp_pipelines::seq

static constexpr inline auto slice = slice_fn{};
}  // namespace cpp_pipelines::seq