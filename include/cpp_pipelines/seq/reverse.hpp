#pragma once

#include <cpp_pipelines/pipeline.hpp>
#include <cpp_pipelines/seq/views.hpp>
#include <cpp_pipelines/subrange.hpp>

namespace cpp_pipelines::seq
{
struct reverse_fn
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
            using reference = iter_reference_t<inner_iterator>;
            inner_iterator it;

            constexpr reference deref() const
            {
                return *std::prev(it);
            }

            constexpr void inc()
            {
                --it;
            }

            constexpr bool is_equal(const iter& other) const
            {
                return it == other.it;
            }

            template <class It = inner_iterator, class = std::enable_if_t<is_bidirectional_iterator<It>::value>>
            constexpr void dec()
            {
                ++it;
            }

            template <class It = inner_iterator, class = std::enable_if_t<is_random_access_iterator<It>::value>>
            constexpr void advance(iter_difference_t<It> offset)
            {
                it -= offset;
            }

            template <class It = inner_iterator, class = std::enable_if_t<is_random_access_iterator<It>::value>>
            constexpr bool is_less(const iter& other) const
            {
                return it > other.it;
            }

            template <class It = inner_iterator, class = std::enable_if_t<is_random_access_iterator<It>::value>>
            constexpr iter_difference_t<It> distance_to(const iter& other) const
            {
                return it - other.it;
            }
        };

        using iterator = iterator_interface<iter>;

        constexpr iterator begin() const
        {
            return { iter{ std::end(range) } };
        }

        constexpr iterator end() const
        {
            return { iter{ std::begin(range) } };
        }
    };

    template <class Range>
    constexpr auto operator()(Range&& range) const
    {
        //return subrange{ make_iterator(std::end(range)), make_iterator(std::begin(range)) };
        return view_interface{ view{ all(std::forward<Range>(range)) } };
    }

private:
    template <class Iter>
    constexpr auto make_iterator(Iter iter) const
    {
        return std::make_reverse_iterator(iter);
    }

    template <class Iter>
    constexpr auto make_iterator(std::reverse_iterator<Iter> iter) const
    {
        return iter.base();
    }
};

static constexpr inline auto reverse = make_pipeline(reverse_fn{});

}  // namespace cpp_pipelines::seq