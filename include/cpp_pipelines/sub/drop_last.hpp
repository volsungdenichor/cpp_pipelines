#pragma once

#include <cpp_pipelines/sub/drop.hpp>
#include <cpp_pipelines/sub/reverse.hpp>

namespace cpp_pipelines::sub
{
namespace detail
{
struct drop_last_fn
{
    constexpr auto operator()(std::ptrdiff_t count) const
    {
        return reverse >>= drop(count) >>= reverse;
    }
};
}  // namespace detail

static constexpr inline auto drop_last = detail::drop_last_fn{};
}  // namespace cpp_pipelines::sub