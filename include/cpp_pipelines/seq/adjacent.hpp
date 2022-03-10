#pragma once

#include <cpp_pipelines/seq/drop.hpp>
#include <cpp_pipelines/seq/zip.hpp>

namespace cpp_pipelines::seq
{
namespace detail
{
template <std::size_t N>
struct adjacent_fn
{
    template <class Range>
    constexpr auto operator()(Range&& range) const
    {
        return call(std::forward<Range>(range), std::make_index_sequence<N>{});
    }

private:
    template <class Range, std::size_t... I>
    constexpr auto call(Range&& range, std::index_sequence<I...>) const
    {
        return zip((range >>= drop(I))...);
    }
};

}  // namespace detail

template <std::size_t N>
static constexpr inline auto adjacent = make_pipeline(detail::adjacent_fn<N>{});
static constexpr inline auto pairwise = adjacent<2>;

}  // namespace cpp_pipelines::seq