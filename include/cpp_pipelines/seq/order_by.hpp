#pragma once

#include <cpp_pipelines/algorithm.hpp>
#include <cpp_pipelines/seq/to.hpp>

namespace cpp_pipelines::seq
{
namespace detail
{
struct order_by_fn
{
    template <class Compare, class Proj>
    struct impl
    {
        Compare compare;
        Proj proj;

        template <class Range>
        constexpr auto operator()(Range&& range) const
        {
            auto result = std::forward<Range>(range) >>= to<std::vector>;
            algorithm::sort(result, compare, proj);
            return result;
        }
    };

    template <class Compare = std::less<>, class Proj = identity_fn>
    constexpr auto operator()(Compare compare = {}, Proj proj = {}) const
    {
        return make_pipeline(impl<Compare, Proj>{ std::move(compare), std::move(proj) });
    }
};
}  // namespace detail

static constexpr inline auto order_by = detail::order_by_fn{};

}  // namespace cpp_pipelines::seq