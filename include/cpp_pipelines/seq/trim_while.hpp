#pragma once

#include <cpp_pipelines/seq/drop_while.hpp>
#include <cpp_pipelines/seq/reverse.hpp>

namespace cpp_pipelines::seq
{
namespace detail
{
struct trim_while_fn
{
    template <class Pred>
    constexpr auto operator()(Pred pred) const
    {
        return drop_while(pred) |= reverse |= drop_while(pred) |= reverse;
    }
};

}  // namespace detail

static constexpr inline auto trim_while = detail::trim_while_fn{};
}  // namespace cpp_pipelines::seq