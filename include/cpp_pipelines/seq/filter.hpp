#pragma once

#include <cpp_pipelines/pipeline.hpp>
#include <cpp_pipelines/semiregular.hpp>
#include <cpp_pipelines/seq/views.hpp>

namespace cpp_pipelines::seq
{
namespace detail
{
struct filter_fn
{
    template <class Pred, class Range>
    struct view
    {
        semiregular<Pred> pred;
        Range range;

        constexpr view(Pred pred, Range range)
            : pred{ std::move(pred) }
            , range{ std::move(range) }
        {
        }

        struct iter
        {
            using inner_iterator = iterator_t<Range>;
            using reference = iter_reference_t<inner_iterator>;
            const view* parent;
            inner_iterator it;

            constexpr iter() = default;

            constexpr iter(const view* parent, inner_iterator it)
                : parent{ parent }
                , it{ it }
            {
                update();
            }

            constexpr reference deref() const
            {
                return *it;
            }

            constexpr void inc()
            {
                ++it;
                update();
            }

            template <class It = inner_iterator, class = std::enable_if_t<is_bidirectional_iterator<It>::value>>
            constexpr void dec()
            {
                --it;
                while (!invoke(parent->pred, *it))
                {
                    --it;
                }
            }

            constexpr bool is_equal(const iter& other) const
            {
                return it == other.it;
            }

            template <class It = inner_iterator, class = std::enable_if_t<is_random_access_iterator<It>::value>>
            constexpr bool is_less(const iter& other) const
            {
                return it < other.it;
            }

        private:
            constexpr void update()
            {
                while (it != std::end(parent->range) && !invoke(parent->pred, *it))
                {
                    ++it;
                }
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
            return view_interface{ view{ pred, all(std::forward<Range>(range)) } };
        }
    };

    template <class Pred>
    constexpr auto operator()(Pred pred) const
    {
        return make_pipeline(impl<Pred>{ std::move(pred) });
    }
};

}  // namespace detail

static constexpr inline auto filter = detail::filter_fn{};

}  // namespace cpp_pipelines::seq