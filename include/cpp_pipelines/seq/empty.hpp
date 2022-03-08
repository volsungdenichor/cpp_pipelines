#pragma once

#include <cpp_pipelines/pipeline.hpp>
#include <cpp_pipelines/type_traits.hpp>
#include <stdexcept>

namespace cpp_pipelines::seq
{
struct empty_fn
{
    template <class Range>
    constexpr bool operator()(Range&& range) const
    {
        return (*this)(std::begin(range), std::end(range));
    }

    template <class Iter>
    constexpr bool operator()(Iter b, Iter e) const
    {
        return b == e;
    }
};

static constexpr inline auto empty = make_pipeline(empty_fn{});

}  // namespace cpp_pipelines::seq