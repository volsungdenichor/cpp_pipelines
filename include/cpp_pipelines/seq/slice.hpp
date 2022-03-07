#pragma once

#include <cpp_pipelines/pipeline.hpp>
#include <cpp_pipelines/seq/drop.hpp>
#include <cpp_pipelines/seq/take.hpp>
#include <cpp_pipelines/subrange.hpp>

namespace cpp_pipelines::seq
{
struct slice_fn
{
    constexpr auto operator()(std::ptrdiff_t offset, std::ptrdiff_t count) const
    {
        return drop(offset) >>= take(count);
    }
};

static constexpr inline auto slice = slice_fn{};

}  // namespace cpp_pipelines::seq