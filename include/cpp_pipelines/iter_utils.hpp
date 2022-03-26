#pragma once

#include <cpp_pipelines/invoke.hpp>
#include <cpp_pipelines/type_traits.hpp>

namespace cpp_pipelines
{
template <class Iter>
constexpr Iter advance(Iter it, iter_difference_t<Iter> n, Iter sentinel)
{
    if constexpr (is_random_access_iterator<Iter>::value)
    {
        return std::next(it, std::min(n, std::distance(it, sentinel)));
    }
    else
    {
        while (n > 0 && it != sentinel)
        {
            ++it;
            --n;
        }
        return it;
    }
}

template <class Iter, class Pred>
constexpr Iter advance_while(Iter it, Pred pred, Iter sentinel)
{
    while (it != sentinel && invoke(pred, *it))
    {
        ++it;
    }
    return it;
}

namespace detail
{
struct iter_find_fn
{
    template <class Pred>
    struct impl
    {
        Pred pred;

        template <class Range>
        constexpr auto operator()(Range&& range) const
        {
            return (*this)(std::begin(range), std::end(range));
        }

    private:
        template <class Iter>
        constexpr Iter operator()(Iter begin, Iter end) const
        {
            return advance_while(begin, std::not_fn(pred), end);
        }
    };

    template <class Pred>
    constexpr auto operator()(Pred pred) const
    {
        return make_pipeline(impl<Pred>{ std::move(pred) });
    }
};

struct iter_at_fn
{
    struct impl
    {
        std::ptrdiff_t n;

        template <class Range>
        constexpr auto operator()(Range&& range) const
        {
            return (*this)(std::begin(range), std::end(range));
        }

    private:
        template <class Iter>
        constexpr Iter operator()(Iter begin, Iter end) const
        {
            return advance(begin, n, end);
        }
    };

    constexpr auto operator()(std::ptrdiff_t n) const
    {
        return make_pipeline(impl{ n });
    }
};

}  // namespace detail

static constexpr inline auto iter_find = detail::iter_find_fn{};
static constexpr inline auto iter_at = detail::iter_at_fn{};

}  // namespace cpp_pipelines