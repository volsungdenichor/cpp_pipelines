#pragma once

#include <cpp_pipelines/seq/drop.hpp>
#include <cpp_pipelines/seq/zip_transform.hpp>

namespace cpp_pipelines::seq
{
namespace detail
{
template <std::size_t N>
struct adjacent_transform_fn
{
    template <class Func>
    struct impl
    {
        Func func;

        constexpr impl(Func func)
            : func{ std::move(func) }
        {
        }

        template <class Range>
        constexpr auto operator()(Range&& range) const
        {
            return call(std::forward<Range>(range), std::make_index_sequence<N>{});
        }

    private:
        template <class Range, std::size_t... I>
        constexpr auto call(Range&& range, std::index_sequence<I...>) const
        {
            return std::tuple{ (range |= drop(I))... } >>= seq::zip_transform(func);
        }
    };

    template <class Func>
    constexpr auto operator()(Func func) const
    {
        return fn(impl<Func>{ std::move(func) });
    }
};

}  // namespace detail

template <std::size_t N>
static constexpr inline auto adjacent_transform = detail::adjacent_transform_fn<N>{};
static constexpr inline auto pairwise_transform = adjacent_transform<2>;

}  // namespace cpp_pipelines::seq