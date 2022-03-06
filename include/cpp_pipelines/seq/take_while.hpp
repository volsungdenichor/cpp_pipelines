#pragma once

#include <cpp_pipelines/iter_utils.hpp>
#include <cpp_pipelines/pipeline.hpp>
#include <cpp_pipelines/subrange.hpp>

namespace cpp_pipelines::seq
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
            auto b = std::begin(range);
            auto e = std::end(range);
            return subrange{ b, advance_while(b, pred, e) };
        }
    };

    template <class Pred>
    constexpr auto operator()(Pred pred) const
    {
        return make_pipeline(impl<Pred>{ std::move(pred) });
    }
};

static constexpr inline auto take_while = take_while_fn{};

}  // namespace cpp_pipelines::seq