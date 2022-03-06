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

}  // namespace cpp_pipelines