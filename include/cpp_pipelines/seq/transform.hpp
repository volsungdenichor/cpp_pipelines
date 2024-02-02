#pragma once

#include <cpp_pipelines/pipeline.hpp>
#include <cpp_pipelines/seq/views.hpp>

namespace cpp_pipelines::seq
{
namespace detail
{
struct transform_fn
{
    template <class Func, class Range>
    struct view
    {
        Func func;
        Range range;

        constexpr view(Func func, Range range)
            : func{ std::move(func) }
            , range{ std::move(range) }
        {
        }

        struct iter
        {
            using inner_iterator = iterator_t<Range>;
            const view* parent;
            inner_iterator it;

            constexpr iter() = default;

            constexpr iter(const view* parent, inner_iterator it)
                : parent{ parent }
                , it{ it }
            {
            }

            constexpr decltype(auto) deref() const
            {
                return to_return_type(invoke(parent->func, *it));
            }

            constexpr void inc()
            {
                ++it;
            }

            constexpr bool is_equal(const iter& other) const
            {
                return it == other.it;
            }

            template <class It = inner_iterator, class = std::enable_if_t<is_bidirectional_iterator<It>::value>>
            constexpr void dec()
            {
                --it;
            }

            template <class It = inner_iterator, class = std::enable_if_t<is_random_access_iterator<It>::value>>
            constexpr void advance(iter_difference_t<It> offset)
            {
                it += offset;
            }

            template <class It = inner_iterator, class = std::enable_if_t<is_random_access_iterator<It>::value>>
            constexpr iter_difference_t<It> distance_to(const iter& other) const
            {
                return other.it - it;
            }
        };

        using iterator = iterator_interface<iter>;

        constexpr iterator begin() const
        {
            return { this, std::begin(range) };
        }

        constexpr iterator end() const
        {
            return { this, std::end(range) };
        }
    };

    template <class Func>
    struct impl
    {
        Func func;

        template <class Range>
        constexpr auto operator()(Range&& range) const
        {
            return view_interface{ view{ func, all(std::forward<Range>(range)) } };
        }
    };

    template <class Func>
    constexpr auto operator()(Func func) const
    {
        return fn(impl<Func>{ std::move(func) });
    }
};

}  // namespace detail

static constexpr inline auto transform = detail::transform_fn{};

}  // namespace cpp_pipelines::seq