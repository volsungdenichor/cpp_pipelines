#pragma once

#include <algorithm>
#include <cpp_pipelines/pipeline.hpp>
#include <iterator>

namespace cpp_pipelines::seq
{
namespace detail
{
struct copy_fn
{
    template <class Iter>
    struct impl
    {
        Iter iter;

        template <class Range>
        constexpr Iter operator()(Range&& range) const
        {
            return std::copy(std::begin(range), std::end(range), iter);
        }
    };

    template <class Iter>
    constexpr auto operator()(Iter iter) const
    {
        return fn(impl<Iter>{ iter });
    }
};

struct push_back_fn
{
    template <class Container>
    constexpr auto operator()(Container& container) const
    {
        return copy_fn{}(std::back_inserter(container));
    }
};

}  // namespace detail

static constexpr inline auto copy = detail::copy_fn{};
static constexpr inline auto push_back = detail::push_back_fn{};

}  // namespace cpp_pipelines::seq