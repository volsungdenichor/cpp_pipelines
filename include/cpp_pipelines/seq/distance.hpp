#pragma once

#include <cpp_pipelines/pipeline.hpp>
#include <cpp_pipelines/type_traits.hpp>
#include <stdexcept>

namespace cpp_pipelines::seq
{
struct distance_fn
{
    template <class Range>
    constexpr decltype(auto) operator()(Range&& range) const
    {
        return (*this)(std::begin(range), std::end(range));
    }

    template <class Iter>
    constexpr auto operator()(Iter b, Iter e) const -> iter_difference_t<Iter>
    {
        return std::distance(b, e);
    }
};

static constexpr inline auto distance = make_pipeline(distance_fn{});
static constexpr inline auto size = distance;

}  // namespace cpp_pipelines::seq