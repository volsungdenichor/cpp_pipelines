#pragma once

#include <cpp_pipelines/opt.hpp>
#include <cpp_pipelines/pipeline.hpp>
#include <cpp_pipelines/seq/drop.hpp>
#include <cpp_pipelines/seq/reverse.hpp>
#include <cpp_pipelines/type_traits.hpp>
#include <stdexcept>

namespace cpp_pipelines::seq
{
struct front_fn
{
    template <class Range>
    constexpr decltype(auto) operator()(Range&& range) const
    {
        return (*this)(std::begin(range), std::end(range));
    }

    template <class Iter>
    constexpr auto operator()(Iter b, Iter e) const -> iter_reference_t<Iter>
    {
        if (b == e)
        {
            throw std::out_of_range{ "iterator out of range" };
        }
        return *b;
    }
};

struct maybe_front_fn
{
    template <class Range>
    constexpr auto operator()(Range&& range) const
    {
        return (*this)(std::begin(range), std::end(range));
    }

    template <class Iter>
    constexpr auto operator()(Iter b, Iter e) const
    {
        return b != e
                   ? opt::to_optional(*b)
                   : std::nullopt;
    }
};

static constexpr inline auto front = make_pipeline(front_fn{});
static constexpr inline auto maybe_front = make_pipeline(maybe_front_fn{});

struct at_fn
{
    constexpr auto operator()(std::ptrdiff_t index) const
    {
        return drop(index) >>= front;
    }
};

struct maybe_at_fn
{
    constexpr auto operator()(std::ptrdiff_t index) const
    {
        return drop(index) >>= maybe_front;
    }
};

static constexpr inline auto at = at_fn{};
static constexpr inline auto maybe_at = maybe_at_fn{};

static constexpr inline auto back = reverse() >>= front;
static constexpr inline auto maybe_back = reverse() >>= maybe_front;

}  // namespace cpp_pipelines::seq