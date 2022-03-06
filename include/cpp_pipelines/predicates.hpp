#pragma once

#include <cpp_pipelines/functions.hpp>
#include <cpp_pipelines/type_traits.hpp>
#include <tuple>

namespace cpp_pipelines::predicates
{
namespace detail
{
template <class T, class U>
using is_equality_comparable = decltype(std::declval<const T&>() == std::declval<const U&>());
}  // namespace detail

template <class Impl>
struct predicate_interface;

template <class T>
struct is_predicate_interface : std::false_type
{
};

template <class Impl>
struct is_predicate_interface<predicate_interface<Impl>> : std::true_type
{
};

template <class T, class Pred>
constexpr bool matches(const T& item, const Pred& pred)
{
    if constexpr (is_detected_v<detail::is_equality_comparable, T, Pred> && !is_predicate_interface<Pred>::value)
    {
        return item == pred;
    }
    else
    {
        return invoke(pred, item);
    }
}

template <class Impl>
struct predicate_interface
{
    Impl impl;

    template <class T>
    constexpr bool operator()(const T& item) const
    {
        return matches(item, impl);
    }
};

template <class Impl>
predicate_interface(Impl) -> predicate_interface<Impl>;

struct placeholder_impl
{
    template <class T>
    constexpr bool operator()(const T&) const
    {
        return true;
    }
};

struct negate_fn
{
    template <class Inner>
    struct impl
    {
        Inner inner;

        template <class T>
        constexpr bool operator()(const T& item) const
        {
            return !matches(item, inner);
        }
    };

    template <class Impl>
    constexpr auto operator()(predicate_interface<Impl> pred) const
    {
        return predicate_interface{ impl<predicate_interface<Impl>>{ std::move(pred) } };
    }

    template <class Impl>
    constexpr auto operator()(predicate_interface<impl<Impl>> pred) const
    {
        return pred.impl.inner;
    }
};

struct all_fn
{
    template <class T, class Preds>
    constexpr bool operator()(const T& item, const Preds& preds) const
    {
        return check(item, preds, std::make_index_sequence<std::tuple_size_v<Preds>>{});
    }

    template <class T, class Preds, std::size_t... I>
    constexpr bool check(const T& item, const Preds& preds, std::index_sequence<I...>) const
    {
        return (... && matches(item, std::get<I>(preds)));
    }
};

struct any_fn
{
    template <class T, class Preds>
    constexpr bool operator()(const T& item, const Preds& preds) const
    {
        return check(item, preds, std::make_index_sequence<std::tuple_size_v<Preds>>{});
    }

    template <class T, class Preds, std::size_t... I>
    constexpr bool check(const T& item, const Preds& preds, std::index_sequence<I...>) const
    {
        return (... || matches(item, std::get<I>(preds)));
    }
};

template <class Tag>
struct compound_fn
{
    template <class... Preds>
    struct impl
    {
        std::tuple<Preds...> preds;

        template <class T>
        constexpr bool operator()(const T& item) const
        {
            const auto check = Tag{};
            return check(item, preds);
        }
    };

    template <class... Preds>
    constexpr auto operator()(Preds... preds) const
    {
        return predicate_interface{ from_tuple(std::tuple_cat(to_tuple(std::move(preds))...)) };
    }

private:
    template <class Pred>
    constexpr auto to_tuple(Pred pred) const -> std::tuple<Pred>
    {
        return { std::move(pred) };
    }

    template <class... Preds>
    constexpr auto to_tuple(predicate_interface<impl<Preds...>> pred) const -> std::tuple<Preds...>
    {
        return std::move(pred.impl.preds);
    }

    template <class... Preds>
    constexpr auto from_tuple(std::tuple<Preds...> tuple) const -> impl<Preds...>
    {
        return { std::move(tuple) };
    }
};

template <class Op>
struct compare_fn
{
    template <class V>
    struct impl
    {
        static constexpr inline auto op = Op{};

        V value;

        template <class T>
        constexpr bool operator()(const T& item) const
        {
            return op(item, value);
        }
    };

    template <class T>
    constexpr auto operator()(T value) const
    {
        return predicate_interface{ impl<T>{ std::move(value) } };
    }
};

using placeholder_t = predicate_interface<placeholder_impl>;

static constexpr inline auto __ = placeholder_t{};

static constexpr inline auto negate = negate_fn{};
static constexpr inline auto all = compound_fn<all_fn>{};
static constexpr inline auto any = compound_fn<any_fn>{};

static constexpr inline auto equal_to = compare_fn<std::equal_to<>>{};
static constexpr inline auto not_equal_to = compare_fn<std::not_equal_to<>>{};
static constexpr inline auto greater = compare_fn<std::greater<>>{};
static constexpr inline auto greater_equal = compare_fn<std::greater_equal<>>{};
static constexpr inline auto less = compare_fn<std::less<>>{};
static constexpr inline auto less_equal = compare_fn<std::less_equal<>>{};

static constexpr inline auto eq = equal_to;
static constexpr inline auto ne = not_equal_to;
static constexpr inline auto gt = greater;
static constexpr inline auto ge = greater_equal;
static constexpr inline auto lt = less;
static constexpr inline auto le = less_equal;

template <class Impl>
constexpr auto operator!(predicate_interface<Impl> pred)
{
    return negate(std::move(pred));
}

template <class L, class R>
constexpr auto operator&&(predicate_interface<L> lhs, predicate_interface<R> rhs)
{
    return all(std::move(lhs), std::move(rhs));
}

template <class L, class R>
constexpr auto operator||(predicate_interface<L> lhs, predicate_interface<R> rhs)
{
    return any(std::move(lhs), std::move(rhs));
}

template <class T>
constexpr auto operator==(placeholder_t, T value)
{
    return eq(std::move(value));
}

template <class T>
constexpr auto operator!=(placeholder_t, T value)
{
    return ne(std::move(value));
}

template <class T>
constexpr auto operator<(placeholder_t, T value)
{
    return lt(std::move(value));
}

template <class T>
constexpr auto operator<=(placeholder_t, T value)
{
    return le(std::move(value));
}

template <class T>
constexpr auto operator>(placeholder_t, T value)
{
    return gt(std::move(value));
}

template <class T>
constexpr auto operator>=(placeholder_t, T value)
{
    return ge(std::move(value));
}

template <class T>
constexpr auto operator==(T value, placeholder_t)
{
    return eq(std::move(value));
}

template <class T>
constexpr auto operator!=(T value, placeholder_t)
{
    return ne(std::move(value));
}

template <class T>
constexpr auto operator<(T value, placeholder_t)
{
    return gt(std::move(value));
}

template <class T>
constexpr auto operator<=(T value, placeholder_t)
{
    return ge(std::move(value));
}

template <class T>
constexpr auto operator>(T value, placeholder_t)
{
    return lt(std::move(value));
}

template <class T>
constexpr auto operator>=(T value, placeholder_t)
{
    return le(std::move(value));
}

template <class L, class R>
constexpr auto operator==(predicate_interface<L> lhs, R rhs)
{
    return std::move(lhs) && eq(std::move(rhs));
}

template <class L, class R>
constexpr auto operator!=(predicate_interface<L> lhs, R rhs)
{
    return std::move(lhs) && ne(std::move(rhs));
}

template <class L, class R>
constexpr auto operator<(predicate_interface<L> lhs, R rhs)
{
    return std::move(lhs) && lt(std::move(rhs));
}

template <class L, class R>
constexpr auto operator<=(predicate_interface<L> lhs, R rhs)
{
    return std::move(lhs) && le(std::move(rhs));
}

template <class L, class R>
constexpr auto operator>(predicate_interface<L> lhs, R rhs)
{
    return std::move(lhs) && gt(std::move(rhs));
}

template <class L, class R>
constexpr auto operator>=(predicate_interface<L> lhs, R rhs)
{
    return std::move(lhs) && ge(std::move(rhs));
}

template <class L, class R>
constexpr auto operator==(L lhs, predicate_interface<R> rhs)
{
    return std::move(rhs) == std::move(lhs);
}

template <class L, class R>
constexpr auto operator!=(L lhs, predicate_interface<R> rhs)
{
    return std::move(rhs) != std::move(lhs);
}

template <class L, class R>
constexpr auto operator<(L lhs, predicate_interface<R> rhs)
{
    return std::move(rhs) > std::move(lhs);
}

template <class L, class R>
constexpr auto operator<=(L lhs, predicate_interface<R> rhs)
{
    return std::move(rhs) >= std::move(lhs);
}

template <class L, class R>
constexpr auto operator>(L lhs, predicate_interface<R> rhs)
{
    return std::move(rhs) < std::move(lhs);
}

template <class L, class R>
constexpr auto operator>=(L lhs, predicate_interface<R> rhs)
{
    return std::move(rhs) <= std::move(lhs);
}

}  // namespace cpp_pipelines::predicates
