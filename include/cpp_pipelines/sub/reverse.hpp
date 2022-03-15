#pragma once

#include <cpp_pipelines/pipeline.hpp>
#include <cpp_pipelines/subrange.hpp>
#include <iterator>

namespace cpp_pipelines::sub
{
namespace detail
{
struct reverse_fn
{
    template <class Range>
    constexpr auto operator()(Range&& range) const
    {
        return (*this)(std::begin(range), std::end(range));
    }

    template <class Iter>
    constexpr auto operator()(Iter begin, Iter end) const
    {
        return subrange{ std::reverse_iterator{ end }, std::reverse_iterator{ begin } };
    };

    template <class Iter>
    constexpr auto operator()(std::reverse_iterator<Iter> begin, std::reverse_iterator<Iter> end) const
    {
        return subrange{ end.base(), begin.base() };
    };
};
}  // namespace detail
static constexpr inline auto reverse = make_pipeline(detail::reverse_fn{});
}  // namespace cpp_pipelines::sub
