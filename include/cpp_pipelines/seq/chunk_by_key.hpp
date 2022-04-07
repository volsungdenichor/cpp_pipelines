#pragma once

#include <cpp_pipelines/iter_utils.hpp>
#include <cpp_pipelines/seq/split.hpp>

namespace cpp_pipelines::seq
{
namespace detail
{
struct chunk_by_key_fn
{
    template <class Func>
    struct policy
    {
        semiregular<Func> func;

        template <class Iter>
        constexpr subrange<Iter> operator()(subrange<Iter> sub) const
        {
            const auto& key = invoke(func, *std::begin(sub));
            const auto b = advance_while(
                std::begin(sub),
                [&](const auto& x) { return invoke(func, x) == key; },
                std::end(sub));
            return subrange{ b, b };
        }
    };

    template <class Func>
    constexpr auto operator()(Func func) const
    {
        return split(policy<Func>{ std::move(func) });
    }
};

}  // namespace detail

static constexpr inline auto chunk_by_key = detail::chunk_by_key_fn{};
}  // namespace cpp_pipelines::seq