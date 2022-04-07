#pragma once

#include <cpp_pipelines/seq/chunk_base.hpp>
#include <cpp_pipelines/iter_utils.hpp>
#include <cpp_pipelines/pipeline.hpp>

namespace cpp_pipelines::seq
{
namespace detail
{
struct chunk_fn
{
    struct policy
    {
        std::ptrdiff_t size;
        std::ptrdiff_t step;

        template <class Iter>
        constexpr subrange<Iter> operator()(subrange<Iter> sub) const
        {
            const auto b = advance(std::begin(sub), size, std::end(sub));
            const auto e = advance(std::begin(sub), step, std::end(sub));
            return subrange{b, e};
        }
    };

    template <class Range>
    struct view : public chunk_view_base<policy, Range>
    {
        using base_type = chunk_view_base<policy, Range>;

        constexpr view(Range range, std::ptrdiff_t size, std::ptrdiff_t step)
            : base_type{policy{ size, step }, std::move(range)}
        {
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