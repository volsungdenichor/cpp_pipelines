#pragma once

#include <cpp_pipelines/seq/drop.hpp>
#include <cpp_pipelines/seq/transform_zip.hpp>

namespace cpp_pipelines::seq
{
template <class Func, class Range>
constexpr auto adjacent_transform_impl(std::integral_constant<std::size_t, 2u>, Func&& func, Range&& range)
{
    return transform_zip(
        func,
        range,
        range >>= drop(1));
}

template <class Func, class Range>
constexpr auto adjacent_transform_impl(std::integral_constant<std::size_t, 3u>, Func&& func, Range&& range)
{
    return transform_zip(
        func,
        range,
        range >>= drop(1),
        range >>= drop(2));
}

template <std::size_t N>
struct adjacent_transform_fn
{
    template <class Func>
    struct impl
    {
        Func func;

        template <class Range>
        constexpr auto operator()(Range&& range) const
        {
            return adjacent_transform_impl(std::integral_constant<std::size_t, N>{}, func, std::forward<Range>(range));
        }
    };

    template <class Func>
    constexpr auto operator()(Func func) const
    {
        return make_pipeline(impl<Func>{ std::move(func) });
    }
};

template <std::size_t N>
static constexpr inline auto adjacent_transform = adjacent_transform_fn<N>{};
static constexpr inline auto pairwise_transform = adjacent_transform<2>;

}  // namespace cpp_pipelines::seq