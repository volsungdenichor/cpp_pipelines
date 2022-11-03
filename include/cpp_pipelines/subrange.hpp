#pragma once

#include <cpp_pipelines/view_interface.hpp>

namespace cpp_pipelines
{
namespace detail
{
template <class Iter>
struct subrange_base
{
    Iter b;
    Iter e;

    constexpr Iter begin() const
    {
        return b;
    }

    constexpr Iter end() const
    {
        return e;
    }
};
}  // namespace detail

template <class Iter>
struct subrange : public view_interface<detail::subrange_base<Iter>>
{
    using base_type = view_interface<detail::subrange_base<Iter>>;

    constexpr subrange()
        : base_type{}
    {
    }

    template <class It>
    constexpr subrange(It b, It e)
        : base_type{ detail::subrange_base<Iter>{ b, e } }
    {
    }

    template <class It>
    constexpr subrange(std::pair<It, It> pair)
        : subrange{ pair.first, pair.second }
    {
    }

    template <class Range>
    constexpr subrange(Range&& range)
        : subrange{ std::begin(range), std::end(range) }
    {
    }
};

template <class Iter>
subrange(Iter, Iter) -> subrange<Iter>;

template <class Range>
subrange(Range &&) -> subrange<iterator_t<Range>>;

template <class Iter>
subrange(std::pair<Iter, Iter>) -> subrange<Iter>;

template <class Iter>
struct is_view_interface<subrange<Iter>> : std::true_type
{
};

template <class Container>
using view = subrange<decltype(std::begin(std::declval<Container&>()))>;

template <class Container>
using const_view = subrange<decltype(std::begin(std::declval<const Container&>()))>;

template <class T>
using span = subrange<T*>;

template <class T>
using const_span = subrange<const T*>;

}  // namespace cpp_pipelines