#pragma once

#include <cpp_pipelines/pipeline.hpp>
#include <cpp_pipelines/seq/views.hpp>

namespace cpp_pipelines::seq
{
struct enumerate_fn
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
            std::ptrdiff_t index;
            inner_iterator it;

            constexpr iter(std::ptrdiff_t index, inner_iterator it)
                : index{ index }
                , it{ it }
            {
            }

            constexpr std::tuple<std::ptrdiff_t, range_reference_t<Range>> deref() const
            {
                return { index, *it };
            }

            constexpr void inc()
            {
                ++index;
                ++it;
            }

            constexpr bool is_equal(const iter& other) const
            {
                return it == other.it;
            }
        };

        using iterator = iterator_interface<iter>;

        constexpr iterator begin() const
        {
            return { iter{ 0, std::begin(range) } };
        }

        constexpr iterator end() const
        {
            return { iter{ -1, std::end(range) } };
        }
    };

    template <class Range>
    constexpr auto operator()(Range&& range) const
    {
        return view_interface{ view{ all(std::forward<Range>(range)) } };
    }
};

static constexpr inline auto enumerate = make_pipeline(enumerate_fn{});

}  // namespace cpp_pipelines::seq