#pragma once

#include <cpp_pipelines/functions.hpp>
#include <cpp_pipelines/seq/transform_zip.hpp>

namespace cpp_pipelines::seq
{
struct zip_fn
{
    template <class... Ranges>
    struct as_tuple_fn
    {
        constexpr as_tuple_fn(Ranges...)
        {
        }

        constexpr std::tuple<range_reference_t<Ranges>...> operator()(range_reference_t<Ranges>... args) const
        {
            return { args... };
        }
    };

    template <class... Ranges>
    constexpr inline auto operator()(Ranges&&... ranges) const
    {
        return transform_zip(as_tuple_fn{ std::forward<Ranges>(ranges)... }, std::forward<Ranges>(ranges)...);
    }
};

static constexpr inline auto zip = zip_fn{};

}  // namespace cpp_pipelines::seq