#pragma once

#include <cpp_pipelines/seq/drop.hpp>
#include <cpp_pipelines/seq/transform_zip.hpp>

namespace cpp_pipelines::seq
{
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
            return call(std::forward<Range>(range), std::make_index_sequence<N>{});
        }

    private:
        template <class Range, std::size_t... I>
        constexpr auto call(Range&& range, std::index_sequence<I...>) const
        {
            return transform_zip(func, (range >>= drop(I))...);
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