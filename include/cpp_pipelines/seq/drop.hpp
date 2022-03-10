#pragma once

#include <cpp_pipelines/iter_utils.hpp>
#include <cpp_pipelines/pipeline.hpp>
#include <cpp_pipelines/seq/views.hpp>

namespace cpp_pipelines::seq
{
struct drop_fn
{
    template <class Range>
    struct view
    {
        Range range;
        std::ptrdiff_t n;

        constexpr view(Range range, std::ptrdiff_t n)
            : range{ std::move(range) }
            , n{ n }
        {
        }

        struct iter
        {
            using inner_iterator = iterator_t<Range>;

            const view* parent;
            mutable inner_iterator it;
            mutable std::ptrdiff_t n;

            constexpr iter(const view* parent, inner_iterator it, std::ptrdiff_t n)
                : it{ it }
                , n{ n }
            {
            }

            constexpr range_reference_t<Range> deref() const
            {
                return *it;
            }

            constexpr void inc()
            {
                ++it;
            }

            constexpr bool is_equal(const iter& other) const
            {
                if (n > 0)
                {
                    it = advance(it, n, std::end(parent->range));
                    n = 0;
                }

                return it == other.it;
            }
        };

        constexpr auto begin() const
        {
            if constexpr (is_random_access_range<Range>::value)
            {
                return advance(std::begin(range), n, std::end(range));
            }
            else
            {
                return iterator_interface{ iter{ this, std::begin(range), n } };
            }
        }

        constexpr auto end() const
        {
            if constexpr (is_random_access_range<Range>::value)
            {
                return std::end(range);
            }
            else
            {
                return iterator_interface{ iter{ this, std::end(range), 0 } };
            }
        }
    };

    struct impl
    {
        std::ptrdiff_t n;

        template <class Range>
        constexpr auto operator()(Range&& range) const
        {
            return view_interface{ view{ all(std::forward<Range>(range)), n } };
        }
    };

    constexpr auto operator()(std::ptrdiff_t n) const
    {
        return make_pipeline(impl{ n });
    }
};

static constexpr inline auto drop = drop_fn{};

}  // namespace cpp_pipelines::seq