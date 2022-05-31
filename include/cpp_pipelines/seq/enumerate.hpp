#pragma once

#include <cpp_pipelines/pipeline.hpp>
#include <cpp_pipelines/seq/views.hpp>

namespace cpp_pipelines::seq
{
namespace detail
{
struct enumerate_fn
{
    template <class Range>
    struct view
    {
        Range range;

        constexpr view(Range range)
            : range{ std::move(range) }
        {
        }
        struct iter
        {
            using inner_iterator = iterator_t<Range>;
            std::ptrdiff_t index;
            inner_iterator it;

            constexpr iter() = default;

            constexpr iter(std::ptrdiff_t index, inner_iterator it)
                : index{ index }
                , it{ it }
            {
            }

            constexpr std::pair<std::ptrdiff_t, range_reference_t<Range>> deref() const
            {
                return { index, *it };
            }

            constexpr void inc()
            {
                ++index;
                ++it;
            }

            constexpr bool is_equal(const iter& other) const
            {
                return it == other.it;
            }

            template <class It = inner_iterator, class = std::enable_if_t<is_random_access_iterator<It>::value>>
            constexpr void dec()
            {
                --index;
                --it;
            }

            template <class It = inner_iterator, class = std::enable_if_t<is_random_access_iterator<It>::value>>
            constexpr bool is_less(const iter& other) const
            {
                return it < other.it;
            }

            template <class It = inner_iterator, class = std::enable_if_t<is_random_access_iterator<It>::value>>
            constexpr void advance(iter_difference_t<It> offset)
            {
                index += offset;
                it += offset;
            }

            template <class It = inner_iterator, class = std::enable_if_t<is_random_access_iterator<It>::value>>
            constexpr iter_difference_t<It> distance_to(const iter& other) const
            {
                return other.it - it;
            }
        };

        constexpr auto begin() const
        {
            return iterator_interface{ iter{ 0, std::begin(range) } };
        }

        constexpr auto end() const
        {
            if constexpr (is_random_access_iterator<typename iter::inner_iterator>::value)
            {
                return iterator_interface{ iter{ std::distance(std::begin(range), std::end(range)), std::end(range) } };
            }
            else
            {
                return iterator_interface{ iter{ std::numeric_limits<std::ptrdiff_t>::max(), std::end(range) } };
            }
        }
    };

    template <class Range>
    constexpr auto operator()(Range&& range) const
    {
        return view_interface{ view{ all(std::forward<Range>(range)) } };
    }
};

}  // namespace detail

static constexpr inline auto enumerate = make_pipeline(detail::enumerate_fn{});

}  // namespace cpp_pipelines::seq