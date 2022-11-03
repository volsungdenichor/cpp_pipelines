#pragma once

#include <cpp_pipelines/sub/drop_last_while.hpp>
#include <cpp_pipelines/sub/drop_while.hpp>

namespace cpp_pipelines::sub
{
namespace detail
{
struct trim_while_fn
{
    template <class Pred>
    constexpr auto operator()(Pred pred) const
    {
        return drop_while(pred) |= drop_last_while(pred);
    }
};
}  // namespace detail

static constexpr inline auto trim_while = detail::trim_while_fn{};
}  // namespace cpp_pipelines::sub