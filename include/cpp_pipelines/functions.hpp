#pragma once

#include <cpp_pipelines/invoke.hpp>

namespace cpp_pipelines
{
namespace detail
{
struct identity_fn
{
    template <class T>
    constexpr T&& operator()(T&& item) const noexcept
    {
        return std::forward<T>(item);
    }
};

struct decay_copy_fn
{
    template <class T>
    constexpr auto operator()(T&& item) const -> std::decay_t<T>
    {
        return std::forward<T>(item);
    }
};

struct copy_fn
{
    template <class T>
    constexpr auto operator()(T&& item) const -> std::decay_t<T>
    {
        return item;
    }
};

struct wrap_fn
{
    template <class T>
    constexpr auto operator()(T& item) const -> std::reference_wrapper<T>
    {
        return item;
    }
};

struct unwrap_fn
{
    template <class T>
    constexpr auto operator()(std::reference_wrapper<T> item) const -> T&
    {
        return item.get();
    }
};

struct tie_fn
{
    template <class... Args>
    constexpr auto operator()(Args&... args) const
    {
        return std::tie(args...);
    }
};

struct make_tuple_fn
{
    template <class... Args>
    constexpr auto operator()(Args&&... args) const
    {
        return std::make_tuple(std::forward<Args>(args)...);
    }
};

struct make_pair_fn
{
    template <class... Args>
    constexpr auto operator()(Args&&... args) const
    {
        return std::pair{ std::forward<Args>(args)... };
    }
};

template <class Policy>
struct to_tuple_fn
{
    template <class... Getters>
    struct impl
    {
        std::tuple<Getters...> getters;

        template <class T>
        constexpr auto operator()(T&& item) const
        {
            return call(std::forward<T>(item), std::index_sequence_for<Getters...>{});
        }

        template <class T, std::size_t... I>
        constexpr auto call(T&& item, std::index_sequence<I...>) const
        {
            const auto policy = Policy{};
            return policy(invoke(std::get<I>(getters), std::forward<T>(item))...);
        }
    };

    template <class... Getters>
    constexpr auto operator()(Getters... getters) const
    {
        return impl<Getters...>{ std::tuple<Getters...>{ std::move(getters)... } };
    }
};

template <std::size_t I>
struct get_element_fn
{
    template <class T>
    constexpr decltype(auto) operator()(T&& item) const
    {
        return std::get<I>(std::forward<T>(item));
    }
};

template <class Type>
struct cast_fn
{
    template <class T>
    constexpr Type operator()(T&& item) const
    {
        return static_cast<Type>(std::forward<T>(item));
    }
};

}  // namespace detail

using detail::identity_fn;

static constexpr inline auto identity = identity_fn{};
static constexpr inline auto decay_copy = detail::decay_copy_fn{};
static constexpr inline auto copy = detail::copy_fn{};

static constexpr inline auto wrap = detail::wrap_fn{};
static constexpr inline auto unwrap = detail::unwrap_fn{};

static constexpr inline auto tie = detail::to_tuple_fn<detail::tie_fn>{};
static constexpr inline auto make_tuple = detail::to_tuple_fn<detail::make_tuple_fn>{};
static constexpr inline auto make_pair = detail::to_tuple_fn<detail::make_pair_fn>{};

template <std::size_t I>
static constexpr inline auto get_element = detail::get_element_fn<I>{};

static constexpr inline auto get_first = get_element<0>;
static constexpr inline auto get_second = get_element<1>;

static constexpr inline auto get_key = get_element<0>;
static constexpr inline auto get_value = get_element<1>;

template <class T>
static constexpr inline auto cast = detail::cast_fn<T>{};

}  // namespace cpp_pipelines