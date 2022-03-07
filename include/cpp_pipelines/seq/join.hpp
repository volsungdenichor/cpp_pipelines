#pragma once

#include <cpp_pipelines/functions.hpp>
#include <cpp_pipelines/seq/transform_join.hpp>

namespace cpp_pipelines::seq
{
static constexpr inline auto join = transform_join(identity);

}  // namespace cpp_pipelines::seq