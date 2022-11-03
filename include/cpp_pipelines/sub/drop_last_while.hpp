#pragma once

#include <cpp_pipelines/sub/drop_while.hpp>
#include <cpp_pipelines/sub/reverse.hpp>

namespace cpp_pipelines::sub
{
namespace detail
{
struct drop_last_while_fn
{
    template <class Pred>
    constexpr auto operator()(Pred pred) const
    {
        return reverse |= drop_while(std::move(pred)) |= reverse;
    }
};
}  // namespace detail

static constexpr inline auto drop_last_while = detail::drop_last_while_fn{};
}  // namespace cpp_pipelines::sub