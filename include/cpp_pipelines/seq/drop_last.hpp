#pragma once

#include <cpp_pipelines/seq/drop.hpp>
#include <cpp_pipelines/seq/reverse.hpp>

namespace cpp_pipelines::seq
{
namespace detail
{
struct drop_last_fn
{
    constexpr auto operator()(std::ptrdiff_t n) const
    {
        return reverse |= drop(n) |= reverse;
    }
};
}  // namespace detail

static constexpr inline auto drop_last = detail::drop_last_fn{};

}  // namespace cpp_pipelines::seq