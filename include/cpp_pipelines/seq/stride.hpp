#pragma once

#include <cpp_pipelines/pipeline.hpp>
#include <cpp_pipelines/seq/views.hpp>

namespace cpp_pipelines::seq
{

namespace detail
{

struct stride_fn
{
    template <class Range>
    struct view
    {
        Range range;
        std::ptrdiff_t step;

        constexpr view(Range range, std::ptrdiff_t step)
            : range{std::move(range)}
            , step{step}
        {
        }

        struct iter
        {
            using inner_iterator = iterator_t<Range>;
            const view* parent;
            inner_iterator it;

            constexpr range_reference_t<Range> deref() const
            {
                return *it;
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
            return iterator_interface{iter{this, std::begin(range)}};
        }

        constexpr auto end() const
        {
            return iterator_interface{iter{this, std::end(range)}};
        }
    };

    struct impl
    {
        std::ptrdiff_t step;

        template <class Range>
        constexpr auto operator()(Range&& range) const
        {
            return view_interface{view{all(std::forward<Range>(range)), step}};
        }
    };

    constexpr auto operator()(std::ptrdiff_t step) const
    {
        return make_pipeline(impl{step});
    }
};

} // namespace detail

static constexpr inline auto stride = detail::stride_fn{};

} // namespace cpp_pipelines::seq