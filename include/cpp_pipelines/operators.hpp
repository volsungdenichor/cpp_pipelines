#pragma once

#include <cpp_pipelines/pipeline.hpp>

namespace cpp_pipelines
{
namespace detail
{
template <class Op>
struct binary_operator
{
    static constexpr inline auto op = Op{};

    template <class L, class R>
    constexpr auto operator()(L&& lhs, R&& rhs) const
    {
        return op(std::forward<L>(lhs), std::forward<R>(rhs));
    }

    template <class T>
    constexpr inline auto bind_left(T value) const
    {
        return make_pipeline([value = std::move(value)](auto&& item) { return op(value, std::forward<decltype(item)>(item)); });
    }

    template <class T>
    constexpr inline auto bind_right(T value) const
    {
        return make_pipeline([value = std::move(value)](auto&& item) { return op(std::forward<decltype(item)>(item), value); });
    }

    template <class T>
    constexpr auto operator()(T value) const
    {
        return bind_right(std::move(value));
    }
};

}  // namespace detail

constexpr auto negate = make_pipeline(std::negate<>{});
constexpr auto logical_not = make_pipeline(std::logical_not<>{});

constexpr auto plus = detail::binary_operator<std::plus<>>{};
constexpr auto minus = detail::binary_operator<std::minus<>>{};
constexpr auto multiplies = detail::binary_operator<std::multiplies<>>{};
constexpr auto divides = detail::binary_operator<std::divides<>>{};
constexpr auto modulus = detail::binary_operator<std::modulus<>>{};

constexpr auto equal_to = detail::binary_operator<std::equal_to<>>{};
constexpr auto not_equal_to = detail::binary_operator<std::not_equal_to<>>{};
constexpr auto less = detail::binary_operator<std::less<>>{};
constexpr auto less_equal = detail::binary_operator<std::less_equal<>>{};
constexpr auto greater = detail::binary_operator<std::greater<>>{};
constexpr auto greater_equal = detail::binary_operator<std::greater_equal<>>{};

constexpr auto logical_and = detail::binary_operator<std::logical_and<>>{};
constexpr auto logical_or = detail::binary_operator<std::logical_or<>>{};

}  // namespace cpp_pipelines