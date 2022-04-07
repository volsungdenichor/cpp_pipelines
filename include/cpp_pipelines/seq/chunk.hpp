#pragma once

#include <cpp_pipelines/iter_utils.hpp>
#include <cpp_pipelines/seq/split.hpp>

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
            return subrange{ b, e };
        }
    };

    constexpr auto operator()(std::ptrdiff_t size) const
    {
        return split(policy{ size, size });
    }
};

struct slide_fn
{
    constexpr auto operator()(std::ptrdiff_t size) const
    {
        return split(chunk_fn::policy{ size, 1 });
    }
};

}  // namespace detail

static constexpr inline auto chunk = detail::chunk_fn{};
static constexpr inline auto slide = detail::slide_fn{};
}  // namespace cpp_pipelines::seq