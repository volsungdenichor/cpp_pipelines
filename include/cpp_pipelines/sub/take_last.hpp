#pragma once

#include <cpp_pipelines/sub/reverse.hpp>
#include <cpp_pipelines/sub/take.hpp>

namespace cpp_pipelines::sub
{
namespace detail
{
struct take_last_fn
{
    constexpr auto operator()(std::ptrdiff_t count) const
    {
        return reverse >>= take(count) >>= reverse;
    }
};
}  // namespace detail

static constexpr inline auto take_last = detail::take_last_fn{};
}  // namespace cpp_pipelines::sub