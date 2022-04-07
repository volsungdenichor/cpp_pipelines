#pragma once

#include <cpp_pipelines/iter_utils.hpp>
#include <cpp_pipelines/seq/split.hpp>

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

    template <class Compare>
    constexpr auto operator()(Compare compare) const
    {
        return split(policy<Compare>{std::move(compare)});
    }
};

}  // namespace detail

static constexpr inline auto chunk_by = detail::chunk_by_fn{};
}  // namespace cpp_pipelines::seq