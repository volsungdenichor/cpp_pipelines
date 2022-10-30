#pragma once

#include <cpp_pipelines/operators.hpp>
#include <cpp_pipelines/seq/access.hpp>
#include <cpp_pipelines/seq/chunk.hpp>
#include <cpp_pipelines/seq/transform.hpp>

namespace cpp_pipelines::seq
{
namespace detail
{
struct unique_by_fn
{
    template <class Compare>
    constexpr auto operator()(Compare compare) const
    {
        return seq::chunk_by(std::move(compare)) >>= seq::transform(seq::front);
    }
};

struct unique_by_key_fn
{
    template <class Func>
    constexpr auto operator()(Func func) const
    {
        return seq::chunk_by_key(std::move(func)) >>= seq::transform(seq::front);
    }
};
}  // namespace detail

#if 0
static constexpr inline auto unique_by = detail::unique_by_fn{};
static constexpr inline auto unique_by_key = detail::unique_by_key_fn{};
static constexpr inline auto unique = unique_by(equal_to);
#endif
}  // namespace cpp_pipelines::seq
