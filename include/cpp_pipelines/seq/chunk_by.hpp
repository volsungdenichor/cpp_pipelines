#pragma once

#include <cpp_pipelines/iter_utils.hpp>
#include <cpp_pipelines/seq/chunk_base.hpp>
#include <cpp_pipelines/pipeline.hpp>

namespace cpp_pipelines::seq
{
namespace detail
{
struct chunk_by_fn
{
    template <class Compare>
    struct policy
    {
        semiregular<Compare> compare;

        template <class Iter>
        constexpr subrange<Iter> operator()(subrange<Iter> sub) const
        {
            const auto& first = *std::begin(sub);
            const auto b = advance_while(std::begin(sub), [&](const auto& x) { return compare(first, x); }, std::end(sub));
            return subrange{b, b};
        }
    };

    template <class Compare, class Range>
    struct view : public chunk_view_base<policy<Compare>, Range>
    {
        using base_type = chunk_view_base<policy<Compare>, Range>;

        constexpr view(Compare compare, Range range)
            : base_type{policy<Compare>{std::move(compare)}, std::move(range)}
        {
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

static constexpr inline auto chunk_by = detail::chunk_by_fn{};
}  // namespace cpp_pipelines::seq