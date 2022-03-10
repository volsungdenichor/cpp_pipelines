#pragma once

#include <cpp_pipelines/pipeline.hpp>
#include <cpp_pipelines/seq/drop.hpp>
#include <cpp_pipelines/seq/take.hpp>
#include <cpp_pipelines/subrange.hpp>

namespace cpp_pipelines::seq
{
namespace detail
{
struct slice_fn
{
    constexpr auto operator()(std::ptrdiff_t start, std::ptrdiff_t stop) const
    {
        return drop(start) >>= take(std::max(std::ptrdiff_t{ 0 }, stop - start));
    }
};
}  // namespace detail

static constexpr inline auto slice = detail::slice_fn{};

}  // namespace cpp_pipelines::seq