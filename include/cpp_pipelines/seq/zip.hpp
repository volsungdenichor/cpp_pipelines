#pragma once

#include <cpp_pipelines/functions.hpp>
#include <cpp_pipelines/seq/zip_transform.hpp>

namespace cpp_pipelines::seq
{
namespace detail
{
struct zip_fn
{
    struct as_tuple_fn
    {
        template <class... Args>
        constexpr std::tuple<sanitized_t<Args>...> operator()(Args&&... args) const
        {
            return { std::forward<Args>(args)... };
        }
    };

    template <class... Ranges>
    constexpr inline auto operator()(Ranges&&... ranges) const
    {
        return zip_transform(as_tuple_fn{})(std::forward<Ranges>(ranges)...);
    }
};

}  // namespace detail

static constexpr inline auto zip = fn(detail::zip_fn{});

}  // namespace cpp_pipelines::seq