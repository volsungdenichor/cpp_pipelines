#pragma once

#include <cpp_pipelines/sub/reverse.hpp>
#include <cpp_pipelines/sub/take_while.hpp>

namespace cpp_pipelines::sub
{
namespace detail
{
struct take_last_while_fn
{
    template <class Pred>
    constexpr auto operator()(Pred pred) const
    {
        return reverse |= take_while(std::move(pred)) |= reverse;
    }
};
}  // namespace detail

static constexpr inline auto take_last_while = detail::take_last_while_fn{};
}  // namespace cpp_pipelines::sub