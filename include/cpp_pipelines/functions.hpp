#pragma once

#include <cpp_pipelines/invoke.hpp>
#include <utility>

namespace cpp_pipelines
{
namespace detail
{
struct identity_fn
{
    template <class T>
    constexpr T operator()(T&& item) const noexcept
    {
        return std::forward<T>(item);
    }
};

struct dereference_fn
{
    template <class T>
    constexpr decltype(auto) operator()(T&& item) const
    {
        return *std::forward<T>(item);
    }
};

struct addressof_fn
{
    template <class T>
    constexpr T* operator()(T& item) const
    {
        return std::addressof(item);
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

struct wrap_ref_fn
{
    template <class T>
    constexpr auto operator()(T& item) const -> std::reference_wrapper<T>
    {
        return item;
    }
};

struct unwrap_ref_fn
{
    template <class T>
    constexpr auto operator()(std::reference_wrapper<T> item) const -> T&
    {
        return item.get();
    }
};

template <template <class...> class Tuple, template <class> class Type>
struct create_tuple_fn
{
    template <class... Args>
    constexpr auto operator()(Args&&... args) const
    {
        return Tuple<Type<Args>...>{ std::forward<Args>(args)... };
    }
};

struct collect_results_fn
{
    template <class Collector, class... Getters>
    struct impl
    {
        Collector collector;
        std::tuple<Getters...> getters;

        template <class T>
        constexpr decltype(auto) operator()(T&& item) const
        {
            return call(std::forward<T>(item), std::index_sequence_for<Getters...>{});
        }

        template <class T, std::size_t... I>
        constexpr decltype(auto) call(T&& item, std::index_sequence<I...>) const
        {
            return collector(invoke(std::get<I>(getters), std::forward<T>(item))...);
        }
    };

    template <class Collector>
    struct inter
    {
        Collector collector;

        template <class... Getters>
        constexpr auto operator()(Getters... getters) const
        {
            return impl<Collector, Getters...>{ collector, std::tuple<Getters...>{ std::move(getters)... } };
        }
    };

    template <class Collector>
    constexpr auto operator()(Collector collector) const
    {
        return inter<Collector>{ std::move(collector) };
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

struct as_const_fn
{
    template <class T>
    constexpr decltype(auto) operator()(T& item) const
    {
        return std::as_const(item);
    }
};

struct associate_fn
{
    template <class Func>
    struct impl
    {
        Func func;

        template <class T>
        constexpr auto operator()(T&& item) const
        {
            using key_type = std::decay_t<T>;
            using value_type = decltype(invoke(func, std::forward<T>(item)));

            return std::pair<key_type, value_type>{ key_type{ item }, invoke(func, std::forward<T>(item)) };
        }
    };

    template <class Func>
    constexpr auto operator()(Func func) const
    {
        return impl<Func>{ std::move(func) };
    }
};

struct hash_fn
{
    template <class T>
    constexpr std::size_t operator()(const T& item) const
    {
        return std::hash<T>{}(item);
    }
};

template <bool M>
struct bind_fn
{
    template <class Func, class Args>
    struct impl
    {
        Func func;
        Args args;

        constexpr impl(Func func, Args args) : func{ std::move(func) }, args{ std::move(args) }
        {
        }

        template <class... CallArgs>
        constexpr decltype(auto) operator()(CallArgs&&... call_args) const
        {
            return call(std::make_index_sequence<std::tuple_size_v<Args>>{}, std::forward<CallArgs>(call_args)...);
        }

    private:
        template <std::size_t... I, class... CallArgs>
        constexpr decltype(auto) call(std::index_sequence<I...>, CallArgs&&... call_args) const
        {
            if constexpr (M)
                return invoke(func, std::get<I>(args)..., std::forward<CallArgs>(call_args)...);
            else
                return invoke(func, std::forward<CallArgs>(call_args)..., std::get<I>(args)...);
        }
    };

    template <class Func, class... Args>
    constexpr auto operator()(Func func, Args... args) const
    {
        return impl{ std::move(func), std::tuple{ std::move(args)... } };
    }
};

struct proj_fn
{
    template <class Proj, class Func>
    struct impl
    {
        Proj proj;
        Func func;

        template <class... Args>
        constexpr decltype(auto) operator()(Args&&... args) const
        {
            return invoke(func, invoke(proj, args)...);
        }
    };

    template <class Proj, class Func>
    constexpr auto operator()(Proj proj, Func func) const
    {
        return impl<Proj, Func>{ std::move(proj), std::move(func) };
    }
};

}  // namespace detail

using detail::identity_fn;

static constexpr inline auto identity = identity_fn{};
static constexpr inline auto dereference = detail::dereference_fn{};
static constexpr inline auto addressof = detail::addressof_fn{};
static constexpr inline auto decay_copy = detail::decay_copy_fn{};

static constexpr inline auto wrap_ref = detail::wrap_ref_fn{};
static constexpr inline auto unwrap_ref = detail::unwrap_ref_fn{};

static constexpr inline auto tie = detail::create_tuple_fn<std::tuple, std::add_lvalue_reference_t>{};
static constexpr inline auto make_tuple = detail::create_tuple_fn<std::tuple, std::decay_t>{};
static constexpr inline auto forward_as_tuple = detail::create_tuple_fn<std::tuple, std::add_rvalue_reference_t>{};
static constexpr inline auto to_tuple = detail::create_tuple_fn<std::tuple, type_identity_t>{};

static constexpr inline auto tie_as_pair = detail::create_tuple_fn<std::pair, std::add_lvalue_reference_t>{};
static constexpr inline auto make_pair = detail::create_tuple_fn<std::pair, std::decay_t>{};
static constexpr inline auto forward_as_pair = detail::create_tuple_fn<std::pair, std::add_rvalue_reference_t>{};
static constexpr inline auto to_pair = detail::create_tuple_fn<std::pair, type_identity_t>{};

static constexpr inline auto collect_results = detail::collect_results_fn{};

template <std::size_t I>
static constexpr inline auto get_element = detail::get_element_fn<I>{};

static constexpr inline auto get_first = get_element<0>;
static constexpr inline auto get_second = get_element<1>;

static constexpr inline auto get_key = get_element<0>;
static constexpr inline auto get_value = get_element<1>;

template <class T>
static constexpr inline auto cast = detail::cast_fn<T>{};

static constexpr inline auto as_const = detail::as_const_fn{};

static constexpr inline auto associate = detail::associate_fn{};

static constexpr inline auto hash = detail::hash_fn{};

static constexpr inline auto bind_front = detail::bind_fn<true>{};
static constexpr inline auto bind_back = detail::bind_fn<false>{};

static constexpr inline auto proj = detail::proj_fn{};

}  // namespace cpp_pipelines