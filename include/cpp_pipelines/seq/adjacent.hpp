#pragma once

#include <cpp_pipelines/seq/drop.hpp>
#include <cpp_pipelines/seq/zip.hpp>

namespace cpp_pipelines::seq
{
template <class Range>
constexpr auto adjacent_impl(std::integral_constant<std::size_t, 2u>, Range&& range)
{
    return zip(
        range,
        range >>= drop(1));
}

template <class Range>
constexpr auto adjacent_impl(std::integral_constant<std::size_t, 3u>, Range&& range)
{
    return zip(
        range,
        range >>= drop(1),
        range >>= drop(2));
}

template <std::size_t N>
struct adjacent_fn
{
    template <class Range>
    constexpr auto operator()(Range&& range) const
    {
        return adjacent_impl(std::integral_constant<std::size_t, N>{}, std::forward<Range>(range));
    }
};

template <std::size_t N>
static constexpr inline auto adjacent = make_pipeline(adjacent_fn<N>{});
static constexpr inline auto pairwise = adjacent<2>;

}  // namespace cpp_pipelines::seq