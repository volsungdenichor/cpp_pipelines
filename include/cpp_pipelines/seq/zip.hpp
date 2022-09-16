#pragma once

#include <cpp_pipelines/functions.hpp>
#include <cpp_pipelines/seq/zip_transform.hpp>

namespace cpp_pipelines::seq
{
namespace detail
{
struct zip_fn
{
    template <class... Ranges>
    struct as_tuple_fn
    {
        constexpr std::tuple<range_reference_t<Ranges>...> operator()(range_reference_t<Ranges>... args) const
        {
            return { args... };
        }
    };

    template <class... Ranges>
    constexpr inline auto operator()(Ranges&&... ranges) const
    {
        return zip_transform(as_tuple_fn<Ranges...>{})(std::forward<Ranges>(ranges)...);
    }
};

}  // namespace detail

static constexpr inline auto zip = make_pipeline(detail::zip_fn{});

}  // namespace cpp_pipelines::seq