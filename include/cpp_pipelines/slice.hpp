#pragma once

#include <algorithm>
#include <cpp_pipelines/iter_utils.hpp>
#include <cpp_pipelines/pipeline.hpp>
#include <cpp_pipelines/subrange.hpp>
#include <optional>

namespace cpp_pipelines
{
namespace detail
{
struct slice_fn
{
    struct impl
    {
        std::optional<std::ptrdiff_t> start;
        std::optional<std::ptrdiff_t> stop;

        template <class Range>
        constexpr auto operator()(Range&& range) const
        {
            return (*this)(std::begin(range), std::end(range));
        }

        template <class Iter>
        constexpr auto operator()(Iter begin, Iter end) const
        {
            const auto size = std::distance(begin, end);
            const auto start_index = start
                                         ? adjust_index(*start, size)
                                         : std::ptrdiff_t{ 0 };
            const auto stop_index = stop
                                        ? adjust_index(*stop, size)
                                        : std::ptrdiff_t{ size };

            const auto b = advance(begin, start_index, end);
            const auto e = advance(b, std::max(std::ptrdiff_t{ 0 }, stop_index - start_index), end);
            return subrange{ b, e };
        };

        std::ptrdiff_t adjust_index(std::ptrdiff_t index, std::ptrdiff_t size) const
        {
            return std::clamp(index >= 0 ? index : index + size, std::ptrdiff_t{ 0 }, size);
        }
    };

    constexpr auto operator()(std::optional<std::ptrdiff_t> start, std::optional<std::ptrdiff_t> stop) const
    {
        return make_pipeline(impl{ start, stop });
    }
};
}  // namespace detail
static constexpr inline auto slice = detail::slice_fn{};
}  // namespace cpp_pipelines