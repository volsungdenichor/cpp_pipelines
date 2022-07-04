#pragma once

#include <algorithm>
#include <cpp_pipelines/pipeline.hpp>
#include <cpp_pipelines/type_traits.hpp>
#include <functional>
#include <iostream>
#include <memory>
#include <sstream>
#include <type_traits>
#include <variant>

namespace cpp_pipelines::predicates
{
namespace detail
{
template <class Impl, class T>
using has_as = decltype(std::declval<const Impl&>().as(std::declval<const T&>()));

template <class Impl>
using has_format = decltype(std::declval<const Impl&>().format(std::declval<std::ostream&>()));

template <class Impl>
using has_ostream_op = decltype(std::declval<std::ostream&>() << std::declval<const Impl&>());

template <class T, class U>
using is_equality_comparable = decltype(std::declval<const T&>() == std::declval<const U&>());

template <class Tuple, std::size_t... I>
void print_tuple(std::ostream& os, const Tuple& tuple, std::string_view separator, std::index_sequence<I...>)
{
    using namespace std::string_view_literals;
    (..., (os << (I == 0 ? ""sv : separator) << std::get<I>(tuple)));
}

template <class Tuple>
void print_tuple(std::ostream& os, const Tuple& tuple, std::string_view separator)
{
    print_tuple(os, tuple, separator, std::make_index_sequence<std::tuple_size_v<Tuple>>{});
}

template <class Iter>
void print_range(std::ostream& os, Iter b, Iter e, std::string_view separator)
{
    for (auto it = b; it != e; ++it)
    {
        if (it != b)
            os << separator;
        os << *it;
    }
}

template <class Range>
void print_range(std::ostream& os, const Range& range)
{
    if constexpr (is_detected_v<has_ostream_op, Range>)
    {
        os << range;
    }
    else
    {
        print_range(os, std::begin(range), std::end(range), ", ");
    }
}

template <class Iter>
Iter advance(Iter b, std::ptrdiff_t n, Iter e)
{
    while (b != e && n)
    {
        ++b;
        --n;
    }
    return b;
}

template <class T, class Pred>
constexpr bool matches(const T& item, const Pred& pred)
{
    if constexpr (std::is_invocable_v<Pred, T>)
    {
        return pred(item);
    }
    else if constexpr (is_detected_v<is_equality_comparable, T, Pred>)
    {
        return item == pred;
    }
    else
    {
        static_assert(always_false<T>, "Implementation should implement () operator or be equality comparable");
    }
}

struct all_tag
{
};

struct any_tag
{
};

struct elementwise_tag
{
};

struct elements_are_tag
{
};
struct has_prefix_tag
{
};
struct has_suffix_tag
{
};

struct has_sub_tag
{
};

struct contains_all_tag
{
};

struct contains_any_tag
{
};

template <class T>
struct printable
{
};

template <class T>
std::ostream& operator<<(std::ostream& os, const printable<T>& item)
{
    item(os);
    return os;
}

#define PREDICATES_V2_DEFINE_PRINTABLE(TYPE, NAME) \
                                                   \
    template <>                                    \
    struct printable<TYPE>                         \
    {                                              \
        void operator()(std::ostream& os) const    \
        {                                          \
            os << NAME;                            \
        }                                          \
    };

PREDICATES_V2_DEFINE_PRINTABLE(std::equal_to<>, "eq")
PREDICATES_V2_DEFINE_PRINTABLE(std::not_equal_to<>, "ne")
PREDICATES_V2_DEFINE_PRINTABLE(std::less<>, "lt")
PREDICATES_V2_DEFINE_PRINTABLE(std::less_equal<>, "le")
PREDICATES_V2_DEFINE_PRINTABLE(std::greater<>, "gt")
PREDICATES_V2_DEFINE_PRINTABLE(std::greater_equal<>, "ge")

PREDICATES_V2_DEFINE_PRINTABLE(all_tag, "all")
PREDICATES_V2_DEFINE_PRINTABLE(any_tag, "any")

PREDICATES_V2_DEFINE_PRINTABLE(elements_are_tag, "elements_are")
PREDICATES_V2_DEFINE_PRINTABLE(has_prefix_tag, "starts_with")
PREDICATES_V2_DEFINE_PRINTABLE(has_suffix_tag, "ends_with")
PREDICATES_V2_DEFINE_PRINTABLE(has_sub_tag, "has_sub")
PREDICATES_V2_DEFINE_PRINTABLE(contains_all_tag, "contains_all")
PREDICATES_V2_DEFINE_PRINTABLE(contains_any_tag, "contains_any")

#undef PREDICATES_V2_DEFINE_PRINTABLE

template <class Inner>
struct alias_impl
{
    Inner inner;
    std::string name;

    template <class T>
    constexpr bool operator()(const T& item) const
    {
        return inner(item);
    }

    template <class T>
    constexpr decltype(auto) as(const T& item) const
    {
        return inner.as(item);
    }

    void format(std::ostream& os) const
    {
        os << name;
    }
};

template <class Impl>
struct predicate_interface
{
    Impl impl;

    constexpr predicate_interface() = default;

    constexpr predicate_interface(const predicate_interface&) = default;

    constexpr predicate_interface(predicate_interface&&) = default;

    constexpr predicate_interface& operator=(predicate_interface other)
    {
        std::swap(impl, other.impl);
        return *this;
    }

    template <class T>
    constexpr decltype(auto) as(const T& item) const
    {
        if constexpr (is_detected_v<has_as, Impl, T>)
        {
            return impl.as(item);
        }
        else
        {
            return item;
        }
    }

    template <class T>
    constexpr bool is(const T& item) const
    {
        return matches(item, impl);
    }

    template <class T>
    constexpr bool operator()(const T& item) const
    {
        return is(item);
    }

    constexpr auto alias(std::string name) const
    {
        return predicate_interface<alias_impl<predicate_interface<Impl>>>{ *this, std::move(name) };
    }

    friend std::ostream& operator<<(std::ostream& os, const predicate_interface& item)
    {
        if constexpr (is_detected_v<has_format, Impl>)
        {
            item.impl.format(os);
        }
        else if constexpr (is_detected_v<has_ostream_op, Impl>)
        {
            os << item.impl;
        }
        else
        {
            os << "<unknown>";
        }
        return os;
    }
};

template <class Impl>
predicate_interface(Impl) -> predicate_interface<Impl>;

template <class T, class Preds, std::size_t... I>
constexpr bool check_compound(elementwise_tag, const T& item, const Preds& preds, std::index_sequence<I...>)
{
    return (... && matches(std::get<I>(item), std::get<I>(preds)));
}

template <class T, class Preds, std::size_t... I>
constexpr bool check_compound(all_tag, const T& item, const Preds& preds, std::index_sequence<I...>)
{
    return (... && matches(item, std::get<I>(preds)));
}

template <class T, class Preds, std::size_t... I>
constexpr bool check_compound(any_tag, const T& item, const Preds& preds, std::index_sequence<I...>)
{
    return (... || matches(item, std::get<I>(preds)));
}

template <class Tag, class T, class... Preds>
constexpr bool check_compound(Tag tag, const T& item, const std::tuple<Preds...>& preds)
{
    return check_compound(tag, item, preds, std::index_sequence_for<Preds...>{});
}

struct placeholder_impl
{
    template <class T>
    constexpr bool operator()(const T&) const
    {
        return true;
    }

    void format(std::ostream& os) const
    {
        os << "__";
    }
};

using placeholder_t = predicate_interface<placeholder_impl>;

static constexpr inline auto __ = placeholder_t{};

template <class Tag>
struct compound_fn
{
    template <class Inner>
    struct impl
    {
        static constexpr inline auto tag = Tag{};
        static constexpr inline auto name = printable<Tag>{};
        Inner inner;

        template <class T>
        constexpr bool operator()(const T& item) const
        {
            return check_compound(tag, item, inner);
        }

        void format(std::ostream& os) const
        {
            os << name << "(";
            print_tuple(os, inner, ", ");
            os << ")";
        }
    };

    template <class T>
    using predicate_type = predicate_interface<impl<T>>;

    template <class... Preds>
    constexpr auto operator()(Preds... preds) const
    {
        return build(std::tuple{ std::move(preds)... });
    }

    template <class L, class R>
    constexpr auto operator()(predicate_type<L> lhs, predicate_type<R> rhs) const
    {
        return build(std::tuple_cat(std::move(lhs.impl.inner), std::move(rhs.impl.inner)));
    }

    template <class L, class... R>
    constexpr auto operator()(predicate_type<L> lhs, R... rhs) const
    {
        return (*this)(std::move(lhs), (*this)(std::move(rhs)...));
    }

private:
    template <class T>
    constexpr auto build(T tuple) const
    {
        return predicate_type<T>{ std::move(tuple) };
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

        void format(std::ostream& os) const
        {
            os << "!(" << inner << ")";
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

static constexpr inline auto all = compound_fn<all_tag>{};
static constexpr inline auto any = compound_fn<any_tag>{};
static constexpr inline auto negate = negate_fn{};

template <class Op>
struct compare_fn
{
    template <class V>
    struct impl
    {
        static constexpr inline auto op = Op{};
        static constexpr inline auto name = printable<Op>{};

        V value;

        template <class T>
        constexpr bool operator()(const T& item) const
        {
            return op(item, value);
        }

        void format(std::ostream& os) const
        {
            os << name << "(" << value << ")";
        }
    };

    template <class T>
    constexpr auto operator()(T value) const
    {
        return predicate_interface{ impl<T>{ std::move(value) } };
    }
};

static constexpr inline auto eq = compare_fn<std::equal_to<>>{};
static constexpr inline auto ne = compare_fn<std::not_equal_to<>>{};
static constexpr inline auto gt = compare_fn<std::greater<>>{};
static constexpr inline auto ge = compare_fn<std::greater_equal<>>{};
static constexpr inline auto lt = compare_fn<std::less<>>{};
static constexpr inline auto le = compare_fn<std::less_equal<>>{};

#pragma region operators

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

#pragma endregion  // operators

struct result_of_fn
{
    template <class Func, class Inner>
    struct impl
    {
        Func func;
        Inner inner;
        std::string name;

        template <class T>
        constexpr bool operator()(const T& item) const
        {
            return inner(std::invoke(func, item));
        }

        template <class T>
        constexpr decltype(auto) as(const T& item) const
        {
            return std::invoke(func, item);
        }

        void format(std::ostream& os) const
        {
            os << "result_of(" << name << ", " << inner << ")";
        }
    };

    template <class Func, class Pred>
    constexpr auto operator()(std::string name, Func func, Pred pred) const
    {
        auto p = predicate_interface{ std::move(pred) };
        return predicate_interface{ impl<Func, decltype(p)>{ std::move(func), std::move(p), std::move(name) } };
    }

    template <class Func, class Pred>
    constexpr auto operator()(Func func, Pred pred) const
    {
        return (*this)("unknown", std::move(func), std::move(pred));
    }
};

struct is_some_fn
{
    template <class Inner>
    struct impl
    {
        Inner inner;

        template <class T>
        constexpr bool operator()(const T& item) const
        {
            return static_cast<bool>(item) && inner(*item);
        }

        template <class T>
        constexpr decltype(auto) as(const T& item) const
        {
            return *item;
        }

        constexpr bool operator()(std::nullopt_t) const
        {
            return false;
        }

        constexpr bool operator()(nullptr_t) const
        {
            return false;
        }

        void format(std::ostream& os) const
        {
            os << "is_some(" << inner << ")";
        }
    };

    template <class Pred>
    constexpr auto operator()(Pred pred) const
    {
        auto p = predicate_interface{ std::move(pred) };
        return predicate_interface{ impl<decltype(p)>{ std::move(p) } };
    }

    constexpr auto operator()() const
    {
        return (*this)(__);
    }
};

struct is_none_fn
{
    struct impl
    {
        template <class T>
        constexpr bool operator()(const T& item) const
        {
            return !static_cast<bool>(item);
        }

        constexpr bool operator()(std::nullopt_t) const
        {
            return true;
        }

        constexpr bool operator()(nullptr_t) const
        {
            return true;
        }

        void format(std::ostream& os) const
        {
            os << "is_none()";
        }
    };

    constexpr auto operator()() const
    {
        return predicate_interface{ impl{} };
    }
};

template <class Type>
struct of_type_fn
{
    template <class Inner>
    struct impl
    {
        Inner inner;

        template <class... Types>
        constexpr bool operator()(const std::variant<Types...>& item) const
        {
            return std::holds_alternative<Type>(item) && inner(std::get<Type>(item));
        }

        template <class... Types>
        constexpr const Type& as(const std::variant<Types...>& item) const
        {
            return std::get<Type>(item);
        }

        template <class T>
        constexpr bool operator()(T* item) const
        {
            const Type* derived = dynamic_cast<const Type*>(item);
            return derived && inner(*derived);
        }

        template <class T>
        constexpr const Type& as(T* item) const
        {
            return *dynamic_cast<const Type*>(item);
        }

        template <class T>
        constexpr bool operator()(const std::unique_ptr<T>& item) const
        {
            return (*this)(item.get());
        }

        template <class T>
        constexpr const Type& as(const std::unique_ptr<T>& item) const
        {
            return (*this).as(item.get());
        }

        template <class T>
        constexpr bool operator()(const std::shared_ptr<T>& item) const
        {
            return (*this)(item.get());
        }

        template <class T>
        constexpr const Type& as(const std::shared_ptr<T>& item) const
        {
            return (*this).as(item.get());
        }

        constexpr bool operator()(std::nullopt_t) const
        {
            return false;
        }

        constexpr bool operator()(nullptr_t) const
        {
            return false;
        }

        void format(std::ostream& os) const
        {
            os << "is(" << typeid(Type).name() << ", " << inner << ")";
        }
    };

    template <class Pred>
    constexpr auto operator()(Pred pred) const
    {
        auto p = predicate_interface{ std::move(pred) };
        return predicate_interface{ impl<decltype(p)>{ std::move(p) } };
    }

    constexpr auto operator()() const
    {
        return (*this)(__);
    }
};

struct elementwise_fn
{
    template <class Inner>
    struct impl
    {
        Inner inner;

        template <class T>
        constexpr bool operator()(const T& item) const
        {
            return check_compound(elementwise_tag{}, item, inner);
        }

        void format(std::ostream& os) const
        {
            os << "fields_are(";
            print_tuple(os, inner, ", ");
            os << ")";
        }

    private:
        template <class T, std::size_t... I>
        constexpr bool check(const T& item, std::index_sequence<I...>) const
        {
            return (... && matches(std::get<I>(item), std::get<I>(inner)));
        }
    };

    template <class... Preds>
    constexpr auto operator()(Preds... preds) const
    {
        return predicate_interface{ impl<std::tuple<Preds...>>{ std::tuple<Preds...>{ std::move(preds)... } } };
    }
};

struct all_of_fn
{
    template <class Inner>
    struct impl
    {
        Inner inner;

        template <class T>
        constexpr bool operator()(const T& item) const
        {
            return std::all_of(std::begin(item), std::end(item), inner);
        }

        void format(std::ostream& os) const
        {
            os << "all_of(" << inner << ")";
        }
    };

    template <class Pred>
    constexpr auto operator()(Pred pred) const
    {
        auto p = predicate_interface{ std::move(pred) };
        return predicate_interface{ impl<decltype(p)>{ std::move(p) } };
    }
};

struct none_of_fn
{
    template <class Inner>
    struct impl
    {
        Inner inner;

        template <class T>
        constexpr bool operator()(const T& item) const
        {
            return std::none_of(std::begin(item), std::end(item), inner);
        }

        void format(std::ostream& os) const
        {
            os << "none_of(" << inner << ")";
        }
    };

    template <class Pred>
    constexpr auto operator()(Pred pred) const
    {
        auto p = predicate_interface{ std::move(pred) };
        return predicate_interface{ impl<decltype(p)>{ std::move(p) } };
    }
};

template <class Range, class FindIterator, class Pred>
constexpr bool element_check(const Range& range, FindIterator find_iterator, Pred pred)
{
    auto b = std::begin(range);
    auto e = std::end(range);
    auto iter = find_iterator(b, e);
    return iter != e && matches(*iter, pred);
}

struct count_of_fn
{
    template <class Inner, class Times>
    struct impl
    {
        Inner inner;
        Times times;

        template <class T>
        bool operator()(const T& item) const
        {
            auto b = std::begin(item);
            auto e = std::end(item);
            return times(std::count_if(b, e, inner));
        }

        void format(std::ostream& os) const
        {
            os << "count_of(" << inner << ", " << times << ")";
        }
    };

    template <class Pred, class Times>
    constexpr auto operator()(Pred pred, Times times) const
    {
        auto p = predicate_interface{ std::move(pred) };
        auto t = predicate_interface{ std::move(times) };
        return predicate_interface{ impl<decltype(p), decltype(t)>{ std::move(p), std::move(t) } };
    }
};

struct is_empty_fn
{
    struct impl
    {
        template <class T>
        constexpr bool operator()(const T& item) const
        {
            return std::begin(item) == std::end(item);
        }

        void format(std::ostream& os) const
        {
            os << "is_empty()";
        }
    };

    constexpr auto operator()() const
    {
        return predicate_interface{ impl{} };
    }
};

struct size_is_fn
{
    template <class Inner>
    struct impl
    {
        Inner inner;

        template <class T>
        constexpr bool operator()(const T& item) const
        {
            return inner(std::distance(std::begin(item), std::end(item)));
        }

        void format(std::ostream& os) const
        {
            os << "size_is(" << inner << ")";
        }
    };

    template <class Pred>
    constexpr auto operator()(Pred pred) const
    {
        auto p = predicate_interface{ std::move(pred) };
        return predicate_interface{ impl<decltype(p)>{ std::move(p) } };
    }
};

template <std::size_t I, class Iter, class Preds>
constexpr bool check_range(Iter b, Iter e, const Preds& preds)
{
    if constexpr (I == std::tuple_size_v<Preds>)
    {
        return b == e;
    }
    else
    {
        return b != e && matches(*b, std::get<I>(preds)) && check_range<I + 1>(std::next(b), e, preds);
    }
}

template <class Iter, class Preds>
constexpr bool check_range(elements_are_tag, Iter b, Iter e, const Preds& preds)
{
    return check_range<0>(b, e, preds);
}

template <class Iter, class Preds>
constexpr bool check_range(has_prefix_tag, Iter b, Iter e, const Preds& preds)
{
    const auto size = std::tuple_size_v<Preds>;
    return check_range<0>(b, advance(b, size, e), preds);
}

template <class Iter, class Preds>
constexpr bool check_range(has_suffix_tag, Iter b, Iter e, const Preds& preds)
{
    const auto size = std::distance(b, e) - std::tuple_size_v<Preds>;
    return check_range<0>(advance(b, size, e), e, preds);
}

template <class Iter, class Preds>
constexpr bool check_range(has_sub_tag, Iter b, Iter e, const Preds& preds)
{
    for (; b != e; ++b)
    {
        if (check_range(has_prefix_tag{}, b, e, preds))
        {
            return true;
        }
    }

    return false;
}

template <class Iter, class PredIter>
constexpr bool check_range(has_prefix_tag, Iter b, Iter e, PredIter pred_b, PredIter pred_e)
{
    return std::search(b, e, pred_b, pred_e) == b;
}

template <class Iter, class Preds, std::size_t... I>
constexpr bool check_range_contains_all(Iter b, Iter e, const Preds& preds, std::index_sequence<I...>)
{
    const auto contains = [&](const auto& p) {
        return std::any_of(b, e, p);
    };
    return (... && contains(predicate_interface{ std::get<I>(preds) }));
}

template <class Iter, class Preds, std::size_t... I>
constexpr bool check_range_contains_any(Iter b, Iter e, const Preds& preds, std::index_sequence<I...>)
{
    const auto contains = [&](const auto& p) {
        return std::any_of(b, e, p);
    };
    return (... || contains(predicate_interface{ std::get<I>(preds) }));
}

template <class Iter, class Preds>
constexpr bool check_range(contains_all_tag, Iter b, Iter e, const Preds& preds)
{
    return check_range_contains_all(b, e, preds, std::make_index_sequence<std::tuple_size_v<Preds>>{});
}

template <class Iter, class Preds>
constexpr bool check_range(contains_any_tag, Iter b, Iter e, const Preds& preds)
{
    return check_range_contains_any(b, e, preds, std::make_index_sequence<std::tuple_size_v<Preds>>{});
}

template <class Iter, class PredIter>
constexpr bool check_range(has_suffix_tag, Iter b, Iter e, PredIter pred_b, PredIter pred_e)
{
    auto size = std::distance(b, e);
    auto pred_size = std::distance(pred_b, pred_e);

    return size >= pred_size && std::equal(pred_b, pred_e, advance(b, size - pred_size, e));
}

template <class Iter, class PredIter>
constexpr bool check_range(has_sub_tag, Iter b, Iter e, PredIter pred_b, PredIter pred_e)
{
    return std::search(b, e, pred_b, pred_e) != e;
}

template <class Iter, class PredIter>
constexpr bool check_range(elements_are_tag, Iter b, Iter e, PredIter pred_b, PredIter pred_e)
{
    return std::equal(b, e, pred_b, pred_e);
}

template <class Iter, class PredIter>
constexpr bool check_range(contains_all_tag, Iter b, Iter e, PredIter pred_b, PredIter pred_e)
{
    return std::all_of(pred_b, pred_e, [&](const auto& pred) {
        return std::any_of(b, e, [&](const auto& item) { return matches(pred, item); });
    });
}

template <class Iter, class PredIter>
constexpr bool check_range(contains_any_tag, Iter b, Iter e, PredIter pred_b, PredIter pred_e)
{
    return std::any_of(pred_b, pred_e, [&](const auto& pred) {
        return std::any_of(b, e, [&](const auto& item) { return matches(pred, item); });
    });
}

template <class Tag, std::size_t N = 0>
struct range_elements_fn
{
    template <class Inner>
    struct impl
    {
        static constexpr inline auto tag = Tag{};
        static constexpr inline auto name = printable<Tag>{};
        Inner inner;

        template <class T>
        constexpr bool operator()(const T& item) const
        {
            return check_range(tag, std::begin(item), std::end(item), inner);
        }

        void format(std::ostream& os) const
        {
            os << name << "(";
            print_tuple(os, inner, ", ");
            os << ")";
        }
    };

    template <class... Preds>
    constexpr auto operator()(Preds... preds) const
    {
        return predicate_interface{ impl<std::tuple<Preds...>>{ std::tuple<Preds...>{ std::move(preds)... } } };
    }
};

template <class Tag>
struct range_elements_fn<Tag, 1>
{
    template <class Pred>
    constexpr auto operator()(Pred pred) const
    {
        return range_elements_fn<Tag, 0>{}(std::move(pred));
    }
};

template <class Tag>
struct range_fn
{
    template <class Inner>
    struct impl
    {
        static constexpr inline auto tag = Tag{};
        static constexpr inline auto name = printable<Tag>{};
        Inner inner;

        template <class T>
        bool operator()(const T& item) const
        {
            return check_range(tag, std::begin(item), std::end(item), std::begin(inner), std::end(inner));
        }

        void format(std::ostream& os) const
        {
            os << name << "(";
            print_range(os, inner);
            os << ")";
        }
    };
    template <class Pred>
    constexpr auto operator()(Pred pred) const
    {
        return predicate_interface{ impl<Pred>{ std::move(pred) } };
    }

    template <class T>
    constexpr auto operator()(std::initializer_list<T> pred) const
    {
        return predicate_interface{ impl<std::initializer_list<T>>{ std::move(pred) } };
    }
};

template <class Tag>
struct is_range_fn
{
    template <class Inner>
    struct impl
    {
        static constexpr inline auto tag = Tag{};
        static constexpr inline auto name = printable<Tag>{};

        Inner inner;

        template <class T>
        constexpr bool operator()(const T& item) const
        {
            return check_range(tag, std::begin(inner), std::end(inner), std::begin(item), std::end(item));
        }

        void format(std::ostream& os) const
        {
            os << name << "(";
            print_range(os, inner);
            os << ")";
        }
    };
    template <class Pred>
    constexpr auto operator()(Pred pred) const
    {
        return predicate_interface{ impl<Pred>{ std::move(pred) } };
    }

    template <class T>
    constexpr auto operator()(std::initializer_list<T> pred) const
    {
        return predicate_interface{ impl<std::initializer_list<T>>{ std::move(pred) } };
    }
};

struct is_any_element_of_range
{
    template <class Inner>
    struct impl
    {
        Inner inner;

        template <class T>
        constexpr bool operator()(const T& item) const
        {
            return std::any_of(std::begin(inner), std::end(inner), [&](const auto& p) { return matches(item, p); });
        }

        void format(std::ostream& os) const
        {
            os << "is_any_of(";
            print_range(os, inner);
            os << ")";
        }
    };
    template <class Pred>
    constexpr auto operator()(Pred pred) const
    {
        return predicate_interface{ impl<Pred>{ std::move(pred) } };
    }
};

struct matches_fn
{
    template <class Pred>
    struct impl
    {
        Pred pred;

        template <class T>
        constexpr bool operator()(T&& item) const
        {
            return matches(std::forward<T>(item), pred);
        }
    };

    template <class Pred>
    constexpr auto operator()(Pred pred) const
    {
        return make_pipeline(impl<Pred>{ std::move(pred) });
    }
};

struct assert_fn
{
    template <class Pred>
    struct impl
    {
        Pred pred;

        template <class T>
        constexpr decltype(auto) operator()(T&& item) const
        {
            if (matches(item, pred))
            {
                return std::forward<T>(item);
            }

            std::stringstream ss;
            ss << "expected: " << pred << ", actual: " << item;
            throw std::runtime_error{ ss.str() };
        }
    };

    template <class Pred>
    constexpr auto operator()(Pred pred) const
    {
        return make_pipeline(impl<Pred>{ std::move(pred) });
    }
};

}  // namespace detail

static constexpr inline auto matches = detail::matches_fn{};
static constexpr inline auto assert = detail::assert_fn{};

template <class T>
using predicate_t = std::function<bool(const T&)>;

using detail::eq;
using detail::ge;
using detail::gt;
using detail::le;
using detail::lt;
using detail::ne;

using detail::all;
using detail::any;
using detail::negate;

using detail::__;
using detail::placeholder_t;

static constexpr inline auto is_some = detail::is_some_fn{};
static constexpr inline auto is_none = detail::is_none_fn{};

template <class T>
static constexpr inline auto of_type = detail::of_type_fn<T>{};

static constexpr inline auto result_of = detail::result_of_fn{};
static constexpr inline auto field = detail::result_of_fn{};
static constexpr inline auto property = detail::result_of_fn{};

static constexpr inline auto elementwise = detail::elementwise_fn{};

static constexpr inline auto is_empty = detail::is_empty_fn{};
static constexpr inline auto size_is = detail::size_is_fn{};

static constexpr inline auto all_elements = detail::all_of_fn{};
static constexpr inline auto no_element = detail::none_of_fn{};

static constexpr inline auto number_of_elements = detail::count_of_fn{};

static constexpr inline auto is_equal_to_elements = detail::range_elements_fn<detail::elements_are_tag>{};
static constexpr inline auto starts_with_elements = detail::range_elements_fn<detail::has_prefix_tag>{};
static constexpr inline auto starts_with_element = detail::range_elements_fn<detail::has_prefix_tag, 1>{};
static constexpr inline auto ends_with_elements = detail::range_elements_fn<detail::has_suffix_tag>{};
static constexpr inline auto ends_with_element = detail::range_elements_fn<detail::has_suffix_tag, 1>{};
static constexpr inline auto contains_element = detail::range_elements_fn<detail::contains_any_tag, 1>{};
static constexpr inline auto contains_subrange_of_elements = detail::range_elements_fn<detail::has_sub_tag>{};
static constexpr inline auto contains_all_elements = detail::range_elements_fn<detail::contains_all_tag>{};
static constexpr inline auto contains_any_element = detail::range_elements_fn<detail::contains_any_tag>{};

static constexpr inline auto is_equal_to = detail::range_fn<detail::elements_are_tag>{};
static constexpr inline auto starts_with_range = detail::range_fn<detail::has_prefix_tag>{};
static constexpr inline auto ends_with_range = detail::range_fn<detail::has_suffix_tag>{};
static constexpr inline auto contains_subrange = detail::range_fn<detail::has_sub_tag>{};
static constexpr inline auto contains_all_elements_from_range = detail::range_fn<detail::contains_all_tag>{};
static constexpr inline auto contains_any_element_from_range = detail::range_fn<detail::contains_any_tag>{};

static constexpr inline auto is_prefix_of_range = detail::is_range_fn<detail::has_prefix_tag>{};
static constexpr inline auto is_suffix_of_range = detail::is_range_fn<detail::has_suffix_tag>{};
static constexpr inline auto is_subrange_of_range = detail::is_range_fn<detail::has_sub_tag>{};

static constexpr inline auto is_any_element_of_range = detail::is_any_element_of_range{};

}  // namespace cpp_pipelines::predicates
