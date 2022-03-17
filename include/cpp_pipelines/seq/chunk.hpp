#pragma once

#include <cpp_pipelines/iter_utils.hpp>
#include <cpp_pipelines/pipeline.hpp>
#include <cpp_pipelines/seq/views.hpp>
#include <cpp_pipelines/subrange.hpp>

namespace cpp_pipelines::seq
{
namespace detail
{
struct chunk_fn
{
    template <class Range>
    struct view
    {
        Range range;
        std::ptrdiff_t size;
        std::ptrdiff_t step;

        constexpr view(Range range, std::ptrdiff_t size, std::ptrdiff_t step)
            : range{ std::move(range) }
            , size{ size }
            , step{ step }
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

            constexpr auto deref() const
            {
                return subrange{ it, advance(it, parent->size, std::end(parent->range)) };
            }

            constexpr void inc()
            {
                it = advance(it, parent->step, std::end(parent->range));
            }

            constexpr bool is_equal(const iter& other) const
            {
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

    struct impl
    {
        std::ptrdiff_t size;
        std::ptrdiff_t step;

        template <class Range>
        constexpr auto operator()(Range&& range) const
        {
            return view_interface{ view{ all(std::forward<Range>(range)), size, step } };
        }
    };

    constexpr auto operator()(std::ptrdiff_t size) const
    {
        return make_pipeline(impl{ size, size });
    }
};

struct slide_fn
{
    constexpr auto operator()(std::ptrdiff_t size) const
    {
        return make_pipeline(chunk_fn::impl{ size, 1 });
    }
};

}  // namespace detail

static constexpr inline auto chunk = detail::chunk_fn{};
static constexpr inline auto slide = detail::slide_fn{};
}  // namespace cpp_pipelines::seq