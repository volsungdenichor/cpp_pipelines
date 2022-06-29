#pragma once

#include <cpp_pipelines/seq/enumerate.hpp>
#include <istream>
#include <string>

namespace cpp_pipelines::seq
{
namespace detail
{
struct getlines_fn
{
    auto operator()(std::istream& is, char delimiter = '\n') const
    {
        return generate([&is, delimiter, line = std::string{}]() mutable -> std::optional<std::string> {
            return std::getline(is, line, delimiter)
                       ? std::optional{ line }
                       : std::nullopt;
        });
    }
};
}  // namespace detail

static constexpr inline auto getlines = detail::getlines_fn{};
}  // namespace cpp_pipelines::seq
