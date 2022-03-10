#pragma once

#include <cpp_pipelines/seq/reverse.hpp>
#include <cpp_pipelines/seq/take.hpp>

namespace cpp_pipelines::seq
{
namespace detail
{
struct take_last_fn
{
    constexpr auto operator()(std::ptrdiff_t n) const
    {
        return reverse >>= take(n) >>= reverse;
    }
};
}  // namespace detail

static constexpr inline auto take_last = detail::take_last_fn{};

}  // namespace cpp_pipelines::seq