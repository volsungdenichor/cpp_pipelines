#pragma once

#include <algorithm>
#include <cpp_pipelines/pipeline.hpp>
#include <cpp_pipelines/subrange.hpp>
#include <tuple>

namespace cpp_pipelines::sub
{
namespace detail
{
struct slice_before_fn
{
    template <class Iter>
    constexpr auto operator()(Iter b, Iter m, Iter e) const
    {
        return subrange{ b, m };
    }
};

struct slice_after_fn
{
    template <class Iter>
    constexpr auto operator()(Iter b, Iter m, Iter e) const
    {
        return subrange{ m, e };
    }
};

struct split_fn
{
    template <class Iter>
    constexpr auto operator()(Iter b, Iter m, Iter e) const
    {
        return std::pair{
            subrange{ b, m },
            subrange{ m, e }
        };
    }
};

template <class Policy>
struct split_at_fn
{
    static constexpr inline auto policy = Policy{};

    template <class Middle>
    struct impl
    {
        Middle middle;

        template <class Range>
        constexpr auto operator()(Range&& range) const
        {
            const auto b = std::begin(range);
            const auto e = std::end(range);
            using Iter = decltype(b);
            const auto m = static_cast<Iter>(middle);

            return policy(b, m, e);
        };
    };

    template <class Middle>
    constexpr auto operator()(Middle middle) const
    {
        return make_pipeline(impl<Middle>{ middle });
    }
};
}  // namespace detail

static constexpr inline auto split_at = detail::split_at_fn<detail::split_fn>{};
static constexpr inline auto slice_before = detail::split_at_fn<detail::slice_before_fn>{};
static constexpr inline auto slice_after = detail::split_at_fn<detail::slice_after_fn>{};

}  // namespace cpp_pipelines::sub