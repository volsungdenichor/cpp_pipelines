#pragma once

#include <cpp_pipelines/pipeline.hpp>
#include <numeric>

namespace cpp_pipelines::seq
{
namespace detail
{
struct accumulate_fn
{
    template <class BinaryFunc, class T>
    struct impl
    {
        BinaryFunc func;
        T init;

        template <class Range>
        constexpr auto operator()(Range&& range) const
        {
            return std::accumulate(std::begin(range), std::end(range), init, std::ref(func));
        }
    };

    template <class BinaryFunc>
    struct impl<BinaryFunc, void>
    {
        BinaryFunc func;

        template <class Range>
        constexpr auto operator()(Range&& range) const
        {
            auto b = std::begin(range);
            auto e = std::end(range);
            if (b == e)
            {
                throw std::runtime_error{ "seq::accumulate: empty range" };
            }
            const auto init = *b;
            return std::accumulate(std::next(b), e, init, std::ref(func));
        }
    };

    template <class BinaryFunc, class T>
    constexpr auto operator()(BinaryFunc func, T init) const
    {
        return make_pipeline(impl<BinaryFunc, T>{ std::move(func), std::move(init) });
    }

    template <class BinaryFunc>
    constexpr auto operator()(BinaryFunc func) const
    {
        return make_pipeline(impl<BinaryFunc, void>{ std::move(func) });
    }
};
}  // namespace detail
static constexpr inline auto acccumulate = detail::accumulate_fn{};
}  // namespace cpp_pipelines::seq