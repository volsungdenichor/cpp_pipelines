#pragma once

#include <cpp_pipelines/seq/views.hpp>

namespace cpp_pipelines::seq
{
struct cache_latest_fn
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
            using cache_type = std::conditional_t<
                std::is_lvalue_reference_v<reference>,
                std::add_pointer_t<reference>,
                std::optional<reference>>;

            inner_iterator it;
            mutable cache_type cache = {};

            constexpr reference deref() const
            {
                if (!cache)
                {
                    if constexpr (std::is_lvalue_reference_v<reference>)
                    {
                        cache = std::addressof(*it);
                    }
                    else
                    {
                        cache = *it;
                    }
                }
                return *cache;
            }

            constexpr void inc()
            {
                ++it;
                invalidate();
            }

            constexpr bool is_equal(const iter& other) const
            {
                return it == other.it;
            }

            template <class It = inner_iterator, class = std::enable_if_t<is_bidirectional_iterator<It>::value>>
            constexpr void dec()
            {
                --it;
                invalidate();
            }

            template <class It = inner_iterator, class = std::enable_if_t<is_random_access_iterator<It>::value>>
            constexpr void advance(iter_difference_t<It> offset)
            {
                it += offset;
                invalidate();
            }

            template <class It = inner_iterator, class = std::enable_if_t<is_random_access_iterator<It>::value>>
            constexpr bool is_less(const iter& other) const
            {
                return it < other.it;
            }

            template <class It = inner_iterator, class = std::enable_if_t<is_random_access_iterator<It>::value>>
            constexpr iter_difference_t<It> distance_to(const iter& other) const
            {
                return other.it - it;
            }

        private:
            void invalidate()
            {
                cache = cache_type{};
            }
        };

        constexpr auto begin() const
        {
            return iterator_interface{ iter{ std::begin(range) } };
        }

        constexpr auto end() const
        {
            return iterator_interface{ iter{ std::end(range) } };
        }
    };

    template <class Range>
    constexpr auto operator()(Range&& range) const
    {
        return view_interface{ view{ all(std::forward<Range>(range)) } };
    }
};

static constexpr inline auto cache_latest = make_pipeline(cache_latest_fn{});

}  // namespace cpp_pipelines::seq