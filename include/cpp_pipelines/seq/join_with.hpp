#pragma once

#include <cpp_pipelines/seq/intersperse.hpp>
#include <cpp_pipelines/seq/join.hpp>

namespace cpp_pipelines::seq
{
namespace detail
{
struct join_with_fn
{
    template <class T>
    constexpr auto operator()(T delimiter) const
    {
        return intersperse(std::move(delimiter)) >>= join;
    }
};

}  // namespace detail

static constexpr inline auto join_with = detail::join_with_fn{};

}  // namespace cpp_pipelines::seq