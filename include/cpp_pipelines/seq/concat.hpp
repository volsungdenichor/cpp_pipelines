#pragma once

#include <cpp_pipelines/pipeline.hpp>
#include <cpp_pipelines/seq/views.hpp>

namespace cpp_pipelines::seq
{
namespace detail
{
struct concat_fn
{
    template <class Range1, class Range2>
    struct view
    {
        Range1 range1;
        Range2 range2;

        constexpr view(Range1 range1, Range2 range2)
            : range1{ std::move(range1) }
            , range2{ std::move(range2) }
        {
        }

        struct iter
        {
            const view* parent;
            iterator_t<Range1> it1;
            iterator_t<Range2> it2;

            constexpr iter() = default;

            constexpr iter(const view* parent, iterator_t<Range1> it1, iterator_t<Range2> it2)
                : parent{ parent }
                , it1{ it1 }
                , it2{ it2 }
            {
            }

            constexpr decltype(auto) deref() const
            {
                return to_return_type(it1 != std::end(parent->range1) ? *it1 : *it2);
            }

            constexpr void inc()
            {
                if (it1 != std::end(parent->range1))
                    ++it1;
                else
                    ++it2;
            }

            constexpr bool is_equal(const iter& other) const
            {
                return std::tie(it1, it2) == std::tie(other.it1, other.it2);
            }
        };

        using iterator = iterator_interface<iter>;

        constexpr iterator begin() const
        {
            return { this, std::begin(range1), std::begin(range2) };
        }

        constexpr iterator end() const
        {
            return { this, std::end(range1), std::end(range2) };
        }
    };

    template <class Range1, class Range2>
    constexpr inline auto operator()(Range1&& range1, Range2&& range2) const
    {
        return view_interface{ view{ all(std::forward<Range1>(range1)), all(std::forward<Range2>(range2)) } };
    }

    template <class Range1, class Range2, class... Ranges>
    constexpr inline auto operator()(Range1&& range1, Range2&& range2, Ranges&&... ranges) const
    {
        return (*this)(
            (*this)(std::forward<Range1>(range1), std::forward<Range2>(range2)),
            std::forward<Ranges>(ranges)...);
    }
};

}  // namespace detail

static constexpr inline auto concat = detail::concat_fn{};

}  // namespace cpp_pipelines::seq