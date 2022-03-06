#pragma once

#include <cpp_pipelines/functions.hpp>
#include <cpp_pipelines/seq/transform_join.hpp>

namespace cpp_pipelines::seq
{
struct join_fn
{
    constexpr auto operator()() const
    {
        return transform_join(identity);
    }
};

static constexpr inline auto join = join_fn{};

}  // namespace cpp_pipelines::seq