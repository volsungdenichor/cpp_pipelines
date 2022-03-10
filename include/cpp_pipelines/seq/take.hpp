#pragma once

#include <cpp_pipelines/iter_utils.hpp>
#include <cpp_pipelines/pipeline.hpp>
#include <cpp_pipelines/seq/views.hpp>

namespace cpp_pipelines::seq
{
namespace detail
{
struct take_fn
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
            inner_iterator it;
            std::ptrdiff_t index;

            constexpr iter(const view* parent, inner_iterator it, std::ptrdiff_t index)
                : parent{ parent }
                , it{ it }
                , index{ index }
            {
            }

            constexpr range_reference_t<Range> deref() const
            {
                return *it;
            }

            constexpr void inc()
            {
                ++it;
                ++index;
            }

            constexpr bool is_equal(const iter& other) const
            {
                return it == other.it || index == other.index;
            }
        };

        constexpr auto begin() const
        {
            if constexpr (is_random_access_range<Range>::value)
            {
                return std::begin(range);
            }
            else
            {
                return iterator_interface{ iter{ this, std::begin(range), 0 } };
            }
        }

        constexpr auto end() const
        {
            if constexpr (is_random_access_range<Range>::value)
            {
                return advance(std::begin(range), n, std::end(range));
            }
            else
            {
                return iterator_interface{ iter{ this, std::end(range), n } };
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

}  // namespace detail

static constexpr inline auto take = detail::take_fn{};

}  // namespace cpp_pipelines::seq