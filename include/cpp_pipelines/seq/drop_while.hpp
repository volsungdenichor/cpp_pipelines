#pragma once

#include <cpp_pipelines/iter_utils.hpp>
#include <cpp_pipelines/pipeline.hpp>
#include <cpp_pipelines/subrange.hpp>

namespace cpp_pipelines::seq
{
namespace detail
{
struct drop_while_fn
{
    template <class Range, class Pred>
    struct view
    {
        Range range;
        Pred pred;

        constexpr view(Range range, Pred pred)
            : range{ std::move(range) }
            , pred{ std::move(pred) }
        {
        }

        constexpr auto begin() const
        {
            return advance_while(std::begin(range), pred, std::end(range));
        }

        constexpr auto end() const
        {
            return std::end(range);
        }
    };

    template <class Pred>
    struct impl
    {
        Pred pred;

        template <class Range>
        constexpr auto operator()(Range&& range) const
        {
            return view_interface{ view{ all(std::forward<Range>(range)), pred } };
        }
    };

    template <class Pred>
    constexpr auto operator()(Pred pred) const
    {
        return make_pipeline(impl<Pred>{ std::move(pred) });
    }
};

}  // namespace detail

static constexpr inline auto drop_while = detail::drop_while_fn{};

}  // namespace cpp_pipelines::seq