#pragma once

#include <cpp_pipelines/pipeline.hpp>
#include <cpp_pipelines/seq/views.hpp>

namespace cpp_pipelines::seq
{
struct filter_fn
{
    template <class Pred, class Range>
    struct view
    {
        Pred pred;
        Range range;

        constexpr view(Pred pred, Range range)
            : pred{ std::move(pred) }
            , range{ std::move(range) }
        {
        }

        struct iter
        {
            using inner_iterator = iterator_t<Range>;
            const view* parent;
            inner_iterator it;

            constexpr iter(const view* parent, inner_iterator it)
                : parent{ parent }
                , it{ it }
            {
            }

            constexpr void update()
            {
                while (it != std::end(parent->range) && !invoke(parent->pred, *it))
                {
                    ++it;
                }
            }

            constexpr range_reference_t<Range> deref() const
            {
                return *it;
            }

            constexpr void inc()
            {
                ++it;
                update();
            }

            constexpr bool is_equal(const iter& other) const
            {
                return it == other.it;
            }
        };

        using iterator = iterator_interface<iter>;

        constexpr iterator begin() const
        {
            return { iter{ this, std::begin(range) } };
        }

        constexpr iterator end() const
        {
            return { iter{ this, std::end(range) } };
        }
    };

    template <class Pred>
    struct impl
    {
        Pred pred;

        template <class Range>
        constexpr auto operator()(Range&& range) const
        {
            return view_interface{ view{ pred, all(std::forward<Range>(range)) } };
        }
    };

    template <class Pred>
    constexpr auto operator()(Pred pred) const
    {
        return make_pipeline(impl<Pred>{ std::move(pred) });
    }
};

static constexpr inline auto filter = filter_fn{};
}  // namespace cpp_pipelines::seq