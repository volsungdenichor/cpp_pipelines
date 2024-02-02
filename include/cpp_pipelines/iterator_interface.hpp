#pragma once

#include <cpp_pipelines/type_traits.hpp>
#include <iterator>

namespace cpp_pipelines
{
namespace detail
{
template <class T>
using has_inc_impl = decltype(std::declval<T>().inc());

template <class T>
using has_dec_impl = decltype(std::declval<T>().dec());

template <class T>
using has_advance_impl = decltype(std::declval<T>().advance(std::declval<convertible_to<std::is_integral>>()));

template <class T>
using has_is_equal_impl = decltype(std::declval<T>().is_equal(std::declval<T>()));

template <class T>
using has_is_less_impl = decltype(std::declval<T>().is_less(std::declval<T>()));

template <class T>
using has_distance_to_impl = decltype(std::declval<T>().distance_to(std::declval<T>()));

}  // namespace detail

template <class T>
static constexpr bool has_inc_v = is_detected_v<detail::has_inc_impl, T>;

template <class T>
static constexpr bool has_dec_v = is_detected_v<detail::has_dec_impl, T>;

template <class T>
static constexpr bool has_advance_v = is_detected_v<detail::has_advance_impl, T>;

template <class T>
static constexpr bool has_is_equal_v = is_detected_v<detail::has_is_equal_impl, T>;

template <class T>
static constexpr bool has_is_less_v = is_detected_v<detail::has_is_less_impl, T>;

template <class T>
static constexpr bool has_distance_to_v = is_detected_v<detail::has_distance_to_impl, T>;

template <class T>
struct pointer_proxy
{
    T item;

    constexpr T* operator->()
    {
        return std::addressof(item);
    }
};

template <class Impl>
struct iterator_interface
{
    Impl impl;

    static_assert(std::is_default_constructible_v<Impl>, "iterator_interface must be default constructible");

    constexpr iterator_interface() = default;

    constexpr iterator_interface(const iterator_interface&) = default;
    constexpr iterator_interface(iterator_interface&&) = default;

    template <class... Args, class = std::enable_if_t<std::is_constructible_v<Impl, Args...>>>
    constexpr iterator_interface(Args&&... args) : impl{ Impl{ std::forward<Args>(args)... } }
    {
    }

    constexpr iterator_interface& operator=(iterator_interface other)
    {
        std::swap(impl, other.impl);
        return *this;
    }

    constexpr decltype(auto) operator*() const
    {
        return impl.deref();
    }

    constexpr auto operator->() const
    {
        decltype(auto) ref = **this;
        using ref_type = decltype(ref);
        if constexpr (std::is_reference_v<ref_type>)
            return std::addressof(ref);
        else
            return pointer_proxy<ref_type>{ std::move(ref) };
    }

    template <class T = Impl, class = std::enable_if_t<has_inc_v<T> || has_advance_v<T>>>
    constexpr iterator_interface& operator++()
    {
        if constexpr (has_inc_v<T>)
            impl.inc();
        else
            *this += 1;

        return *this;
    }

    template <class T = Impl, class = std::enable_if_t<has_inc_v<T> || has_advance_v<T>>>
    constexpr iterator_interface operator++(int)
    {
        iterator_interface temp{ *this };
        ++(*this);
        return temp;
    }

    template <class T = Impl, class = std::enable_if_t<has_dec_v<T> || has_advance_v<T>>>
    constexpr iterator_interface& operator--()
    {
        if constexpr (has_dec_v<T>)
            impl.dec();
        else
            *this -= 1;
        return *this;
    }

    template <class T = Impl, class = std::enable_if_t<has_dec_v<T> || has_advance_v<T>>>
    constexpr iterator_interface operator--(int)
    {
        iterator_interface temp{ *this };
        --(*this);
        return temp;
    }

    template <
        class D,
        class T = Impl,
        class = std::enable_if_t<std::is_integral_v<D>>,
        class = std::enable_if_t<has_advance_v<T>>>
    constexpr friend iterator_interface& operator+=(iterator_interface& it, D offset)
    {
        it.impl.advance(offset);
        return it;
    }

    template <
        class D,
        class T = Impl,
        class = std::enable_if_t<std::is_integral_v<D>>,
        class = std::enable_if_t<has_advance_v<T>>>
    constexpr friend iterator_interface operator+(iterator_interface it, D offset)
    {
        return it += offset;
    }

    template <
        class D,
        class T = Impl,
        class = std::enable_if_t<std::is_integral_v<D>>,
        class = std::enable_if_t<has_advance_v<T>>>
    constexpr friend iterator_interface& operator-=(iterator_interface& it, D offset)
    {
        return it += -offset;
    }

    template <
        class D,
        class T = Impl,
        class = std::enable_if_t<std::is_integral_v<D>>,
        class = std::enable_if_t<has_advance_v<T>>>
    constexpr friend iterator_interface operator-(iterator_interface it, D offset)
    {
        return it -= offset;
    }

    template <
        class D,
        class T = Impl,
        class = std::enable_if_t<std::is_integral_v<D>>,
        class = std::enable_if_t<has_advance_v<T>>>
    constexpr decltype(auto) operator[](D offset) const
    {
        return *(*this + offset);
    }

    template <class T = Impl, class = std::enable_if_t<has_distance_to_v<T>>>
    constexpr friend auto operator-(const iterator_interface& lhs, const iterator_interface& rhs)
    {
        return rhs.impl.distance_to(lhs.impl);
    }

    template <class T = Impl, class = std::enable_if_t<has_is_equal_v<T> || has_distance_to_v<T>>>
    constexpr friend bool operator==(const iterator_interface& lhs, const iterator_interface& rhs)
    {
        if constexpr (has_is_equal_v<T>)
            return lhs.impl.is_equal(rhs.impl);
        else
            return lhs.impl.distance_to(rhs.impl) == 0;
    }

    template <class T = Impl, class = std::enable_if_t<has_is_equal_v<T> || has_distance_to_v<T>>>
    constexpr friend bool operator!=(const iterator_interface& lhs, const iterator_interface& rhs)
    {
        return !(lhs == rhs);
    }

    template <class T = Impl, class = std::enable_if_t<has_is_less_v<T> || has_distance_to_v<T>>>
    constexpr friend bool operator<(const iterator_interface& lhs, const iterator_interface& rhs)
    {
        if constexpr (has_is_less_v<T>)
            return lhs.impl.is_less(rhs.impl);
        else
            return lhs.impl.distance_to(rhs.impl) > 0;
    }

    template <class T = Impl, class = std::enable_if_t<has_is_less_v<T> || has_distance_to_v<T>>>
    constexpr friend bool operator>(const iterator_interface& lhs, const iterator_interface& rhs)
    {
        return rhs < lhs;
    }

    template <class T = Impl, class = std::enable_if_t<has_is_less_v<T> || has_distance_to_v<T>>>
    constexpr friend bool operator<=(const iterator_interface& lhs, const iterator_interface& rhs)
    {
        return !(lhs > rhs);
    }

    template <class T = Impl, class = std::enable_if_t<has_is_less_v<Impl> || has_distance_to_v<T>>>
    constexpr friend bool operator>=(const iterator_interface& lhs, const iterator_interface& rhs)
    {
        return !(lhs < rhs);
    }
};

template <class Impl>
iterator_interface(Impl) -> iterator_interface<Impl>;

namespace detail
{
template <class T, class = std::void_t<>>
struct difference_type_impl
{
    using type = std::ptrdiff_t;
};

template <class T>
struct difference_type_impl<T, std::void_t<std::enable_if_t<has_distance_to_v<T>>>>
{
    using type = decltype(std::declval<T>().distance_to(std::declval<T>()));
};

template <class T, class = std::void_t<>>
struct iterator_category_impl
{
    using type = std::conditional_t<
        has_advance_v<T> && has_distance_to_v<T>,
        std::random_access_iterator_tag,
        std::conditional_t<has_dec_v<T> || has_advance_v<T>, std::bidirectional_iterator_tag, std::forward_iterator_tag>>;
};

template <class T>
struct iterator_category_impl<T, std::void_t<typename T::iterator_category>>
{
    using type = typename T::iterator_category;
};

}  // namespace detail

}  // namespace cpp_pipelines

template <class Impl>
struct std::iterator_traits<::cpp_pipelines::iterator_interface<Impl>>
{
    using it = ::cpp_pipelines::iterator_interface<Impl>;
    using reference = decltype(std::declval<it>().operator*());
    using pointer = decltype(std::declval<it>().operator->());
    using value_type = std::decay_t<reference>;
    using difference_type = typename ::cpp_pipelines::detail::difference_type_impl<Impl>::type;
    using iterator_category = typename ::cpp_pipelines::detail::iterator_category_impl<Impl>::type;
};
