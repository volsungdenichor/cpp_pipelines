#pragma once

#include <cpp_pipelines/iter_utils.hpp>
#include <cpp_pipelines/pipeline.hpp>
#include <cpp_pipelines/seq/views.hpp>
namespace cpp_pipelines::seq
{
namespace detail
{
struct take_while_fn
{
    template <class Range, class Pred>
    struct view
    {
        Range range;
        Pred pred;

        constexpr view(Range range, Pred pred)
            : range{ std::move(range) }
            , pred{ std::move(pred) }
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
                return it == other.it || !invoke(parent->pred, *it);
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

    template <class Pred>
    struct impl
    {
        Pred pred;

        template <class Range>
        constexpr auto operator()(Range&& range) const
        {
            return view_interface{ view{ all(std::forward<Range>(range)), pred } };
        }
    };

    template <class Pred>
    constexpr auto operator()(Pred pred) const
    {
        return fn(impl<Pred>{ std::move(pred) });
    }
};

}  // namespace detail

static constexpr inline auto take_while = detail::take_while_fn{};

}  // namespace cpp_pipelines::seq