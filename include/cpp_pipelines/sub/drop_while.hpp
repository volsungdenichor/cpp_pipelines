#pragma once

#include <cpp_pipelines/iter_utils.hpp>
#include <cpp_pipelines/pipeline.hpp>
#include <cpp_pipelines/subrange.hpp>

namespace cpp_pipelines::sub
{
namespace detail
{
struct drop_while_fn
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

            return subrange{ advance_while(b, pred, e), e };
        }
    };

    template <class Pred>
    constexpr auto operator()(Pred pred) const
    {
        return fn(impl<Pred>{ std::move(pred) });
    }
};
}  // namespace detail

static constexpr inline auto drop_while = detail::drop_while_fn{};
}  // namespace cpp_pipelines::sub