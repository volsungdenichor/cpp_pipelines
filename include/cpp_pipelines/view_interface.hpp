#pragma once

#include <cpp_pipelines/iterator_interface.hpp>

namespace cpp_pipelines
{
template <class Impl>
struct view_interface
{
    Impl impl;

    using begin_iterator = decltype(std::declval<const Impl&>().begin());
    using end_iterator = decltype(std::declval<const Impl&>().end());

    using iterator = begin_iterator;
    using reference = typename std::iterator_traits<iterator>::reference;
    using difference_type = typename std::iterator_traits<iterator>::difference_type;

    static_assert(std::is_same_v<begin_iterator, end_iterator>, "begin and end must return the same type of iterator");
    static_assert(is_input_iterator<iterator>::value, "iterator type required");

    constexpr iterator begin() const
    {
        return impl.begin();
    }

    constexpr iterator end() const
    {
        return impl.end();
    }

    template <class Container, class = std::enable_if_t<std::is_constructible_v<Container, iterator, iterator>>>
    constexpr operator Container() const
    {
        return { begin(), end() };
    }

    constexpr bool empty() const
    {
        return begin() == end();
    }

    constexpr reference front() const
    {
        return *begin();
    }

    template <class It = iterator, class = std::enable_if_t<is_bidirectional_iterator<It>::value>>
    constexpr reference back() const
    {
        return *std::prev(end());
    }

    template <class It = iterator, class = std::enable_if_t<is_random_access_iterator<It>::value>>
    constexpr reference operator[](difference_type index) const
    {
        return *std::next(begin(), index);
    }

    template <class It = iterator, class = std::enable_if_t<is_random_access_iterator<It>::value>>
    constexpr reference at(difference_type index) const
    {
        if (0 <= index && index < size())
        {
            return *std::next(begin(), index);
        }
        throw std::out_of_range{};
    }

    constexpr explicit operator bool() const
    {
        return !empty();
    }

#if 0
    constexpr reference operator*() const
    {
        return front();
    }
#endif

    constexpr difference_type size() const
    {
        return std::distance(begin(), end());
    }
};

template <class Impl>
view_interface(Impl) -> view_interface<Impl>;

template <class T>
struct is_view_interface : std::false_type
{
};

template <class Impl>
struct is_view_interface<view_interface<Impl>> : std::true_type
{
};

}  // namespace cpp_pipelines
