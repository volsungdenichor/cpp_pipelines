#pragma once

#include <cpp_pipelines/algorithm.hpp>
#include <cpp_pipelines/pipeline.hpp>

namespace cpp_pipelines::seq
{
namespace detail
{
struct all_of_fn
{
    template <class Pred, class Range>
    constexpr bool operator()(Pred&& pred, Range&& range) const
    {
        return algorithm::all_of(std::forward<Range>(range), std::forward<Pred>(pred));
    }
};

struct any_of_fn
{
    template <class Pred, class Range>
    constexpr bool operator()(Pred&& pred, Range&& range) const
    {
        return algorithm::any_of(std::forward<Range>(range), std::forward<Pred>(pred));
    }
};

struct none_of_fn
{
    template <class Pred, class Range>
    constexpr bool operator()(Pred&& pred, Range&& range) const
    {
        return algorithm::none_of(std::forward<Range>(range), std::forward<Pred>(pred));
    }
};

template <class Policy>
struct check_element_fn
{
    template <class Pred>
    struct impl
    {
        Pred pred;

        template <class Range>
        constexpr bool operator()(Range&& range) const
        {
            const auto policy = Policy{};
            return policy(pred, std::forward<Range>(range));
        }
    };

    template <class Pred>
    constexpr auto operator()(Pred pred) const
    {
        return fn(impl<Pred>{ std::move(pred) });
    }
};

}  // namespace detail

static constexpr inline auto all_of = detail::check_element_fn<detail::all_of_fn>{};
static constexpr inline auto any_of = detail::check_element_fn<detail::any_of_fn>{};
static constexpr inline auto none_of = detail::check_element_fn<detail::none_of_fn>{};

}  // namespace cpp_pipelines::seq
