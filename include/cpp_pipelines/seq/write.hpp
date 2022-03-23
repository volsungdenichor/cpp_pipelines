#pragma once

#include <cpp_pipelines/output.hpp>
#include <cpp_pipelines/seq/copy.hpp>

namespace cpp_pipelines::seq
{
namespace detail
{
struct write_fn
{
    auto operator()(std::ostream& os, std::string_view separator) const
    {
        return copy(ostream_iterator{ os, separator });
    }
};
}  // namespace detail

static constexpr inline auto write = detail::write_fn{};
}  // namespace cpp_pipelines::seq