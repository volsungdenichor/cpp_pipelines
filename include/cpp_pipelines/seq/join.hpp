#pragma once

#include <cpp_pipelines/functions.hpp>
#include <cpp_pipelines/seq/intersperse.hpp>
#include <cpp_pipelines/seq/join.hpp>
#include <cpp_pipelines/seq/transform_join.hpp>

namespace cpp_pipelines::seq
{
namespace detail
{
struct join_with_fn
{
    template <class T>
    constexpr auto operator()(T delimiter) const
    {
        return intersperse(std::move(delimiter)) |= transform_join(identity);
    }
};

}  // namespace detail

static constexpr inline auto join = transform_join(identity);
static constexpr inline auto join_with = detail::join_with_fn{};

}  // namespace cpp_pipelines::seq