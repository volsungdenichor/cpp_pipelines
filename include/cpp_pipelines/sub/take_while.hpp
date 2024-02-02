#pragma once

#include <cpp_pipelines/iter_utils.hpp>
#include <cpp_pipelines/pipeline.hpp>
#include <cpp_pipelines/subrange.hpp>

namespace cpp_pipelines::sub
{
namespace detail
{
struct take_while_fn
{
    template <class Pred>
    struct impl
    {
        Pred pred;

        template <class Range>
        constexpr auto operator()(Range&& range) const
        {
            const auto b = std::begin(range);
            const auto e = std::end(range);

            return subrange{ b, advance_while(b, pred, e) };
        }
    };

    template <class Pred>
    constexpr auto operator()(Pred pred) const
    {
        return fn(impl<Pred>{ std::move(pred) });
    }
};
}  // namespace detail

static constexpr inline auto take_while = detail::take_while_fn{};
}  // namespace cpp_pipelines::sub