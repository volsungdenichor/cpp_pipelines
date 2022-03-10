#pragma once

#include <cpp_pipelines/iter_utils.hpp>
#include <cpp_pipelines/pipeline.hpp>
#include <cpp_pipelines/subrange.hpp>

namespace cpp_pipelines::seq
{
struct drop_while_fn
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
            mutable inner_iterator it;
            mutable bool dirty;

            constexpr iter(const view* parent, inner_iterator it)
                : parent{ parent }
                , it{ it }
                , dirty{ true }
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
                if (dirty)
                {
                    it = advance_while(it, parent->pred, std::end(parent->range));
                    dirty = false;
                }
                return it == other.it;
            }
        };

        constexpr auto begin() const
        {
            return iterator_interface{ iter{ this, std::begin(range) } };
        }

        constexpr auto end() const
        {
            return iterator_interface{ iter{ this, std::end(range) } };
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
        return make_pipeline(impl<Pred>{ std::move(pred) });
    }
};

static constexpr inline auto drop_while = drop_while_fn{};

}  // namespace cpp_pipelines::seq