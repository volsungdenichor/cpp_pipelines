#pragma once

#include <cpp_pipelines/functions.hpp>
#include <cpp_pipelines/seq/transform_zip.hpp>

namespace cpp_pipelines::seq
{
namespace detail
{
struct zip_fn
{
    struct as_tuple_fn
    {
        template <class... Args>
        constexpr auto operator()(Args&&... args) const
        {
            return std::forward_as_tuple(std::forward<Args>(args)...);
        }
    };

    template <class... Ranges>
    constexpr inline auto operator()(Ranges&&... ranges) const
    {
        return transform_zip(as_tuple_fn{}, std::forward<Ranges>(ranges)...);
    }
};

}  // namespace detail

static constexpr inline auto zip = detail::zip_fn{};

}  // namespace cpp_pipelines::seq