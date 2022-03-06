#pragma once

#include <cpp_pipelines/seq/cache_latest.hpp>
#include <cpp_pipelines/seq/join.hpp>
#include <cpp_pipelines/seq/transform.hpp>

namespace cpp_pipelines::seq
{
struct transform_join_fn
{
    template <class Func>
    constexpr auto operator()(Func func) const
    {
        return transform(std::move(func)) >>= cache_latest() >>= join();
    }
};

static constexpr inline auto transform_join = transform_join_fn{};

}  // namespace cpp_pipelines::seq