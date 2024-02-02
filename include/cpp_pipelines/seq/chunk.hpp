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

struct chunk_by_fn
{
    template <class Compare>
    struct policy
    {
        Compare compare;

        template <class Iter>
        constexpr subrange<Iter> operator()(subrange<Iter> sub) const
        {
            const auto& first = *std::begin(sub);
            const auto b = advance_while(
                std::begin(sub),
                [&](const auto& x)
                { return compare(first, x); },
                std::end(sub));
            return subrange{ b, b };
        }
    };

    template <class Compare>
    constexpr auto operator()(Compare compare) const
    {
        return split(policy<Compare>{ std::move(compare) });
    }
};

struct chunk_by_key_fn
{
    template <class Func>
    struct policy
    {
        Func func;

        template <class Iter>
        constexpr subrange<Iter> operator()(subrange<Iter> sub) const
        {
            const auto& key = invoke(func, *std::begin(sub));
            const auto b = advance_while(
                std::begin(sub),
                [&](const auto& x)
                { return invoke(func, x) == key; },
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

static constexpr inline auto chunk = detail::chunk_fn{};
static constexpr inline auto slide = detail::slide_fn{};

static constexpr inline auto chunk_by = detail::chunk_by_fn{};
static constexpr inline auto chunk_by_key = detail::chunk_by_key_fn{};

}  // namespace cpp_pipelines::seq