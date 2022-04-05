#pragma once

#include <cpp_pipelines/pipeline.hpp>
#include <cpp_pipelines/seq/views.hpp>
#include <cpp_pipelines/subrange.hpp>

namespace cpp_pipelines::seq
{
namespace detail
{
struct group_by_fn
{
    template <class Compare, class Range>
    struct view
    {
        semiregular<Compare> compare;
        Range range;

        constexpr view(Compare compare, Range range)
            : compare{ std::move(compare) }
            , range{ std::move(range) }
        {
        }

        struct iter
        {
            using inner_iterator = iterator_t<Range>;
            const view* parent;
            subrange<inner_iterator> current;

            constexpr iter() = default;

            constexpr iter(const view* parent, inner_iterator it)
                : parent{ parent }
                , current{ next(it) }
            {
            }

            constexpr auto deref() const
            {
                return current;
            }

            constexpr void inc()
            {
                current = next(current.end());
            }

            constexpr bool is_equal(const iter& other) const
            {
                return current.begin() == other.current.begin();
            }

        private:
            subrange<inner_iterator> next(inner_iterator it) const
            {
                inner_iterator prev = it;
                while (it != std::end(parent->range) && invoke(parent->compare, *prev, *it))
                {
                    ++it;
                }
                return { prev, it };
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

    template <class Compare>
    struct impl
    {
        Compare compare;

        template <class Range>
        constexpr auto operator()(Range&& range) const
        {
            return view_interface{ view{ compare, all(std::forward<Range>(range)) } };
        }
    };

    template <class Compare>
    constexpr auto operator()(Compare compare) const
    {
        return make_pipeline(impl<Compare>{ std::move(compare) });
    }
};

}  // namespace detail

static constexpr inline auto group_by = detail::group_by_fn{};
}  // namespace cpp_pipelines::seq