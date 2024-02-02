#pragma once

#include <cpp_pipelines/iter_utils.hpp>
#include <cpp_pipelines/pipeline.hpp>
#include <cpp_pipelines/seq/views.hpp>

namespace cpp_pipelines::seq
{
namespace detail
{
struct drop_fn
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

        using iterator = iterator_t<Range>;

        constexpr iterator begin() const
        {
            return advance(std::begin(range), n, std::end(range));
        }

        constexpr iterator end() const
        {
            return std::end(range);
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

static constexpr inline auto drop = detail::drop_fn{};

}  // namespace cpp_pipelines::seq