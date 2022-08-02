#pragma once

#include <cpp_pipelines/algorithm.hpp>
#include <set>
#include <unordered_set>

namespace cpp_pipelines::set
{
template <class L, class R>
bool includes(L&& lhs, R&& rhs)
{
    return algorithm::includes(std::forward<L>(lhs), std::forward<R>(rhs));
}

template <template <class> class Set = std::set, class L, class R>
auto sum(L&& lhs, R&& rhs)
{
    using T = std::common_type_t<range_value_t<L>, range_value_t<R>>;
    Set<T> result;
    algorithm::set_union(std::forward<L>(lhs), std::forward<R>(rhs), std::inserter(result, result.end()));
    return result;
}

template <template <class> class Set = std::set, class L, class R>
auto difference(L&& lhs, R&& rhs)
{
    using T = std::common_type_t<range_value_t<L>, range_value_t<R>>;
    Set<T> result;
    algorithm::set_difference(std::forward<L>(lhs), std::forward<R>(rhs), std::inserter(result, result.end()));
    return result;
}

template <template <class> class Set = std::set, class L, class R>
auto intersection(L&& lhs, R&& rhs)
{
    using T = std::common_type_t<range_value_t<L>, range_value_t<R>>;
    Set<T> result;
    algorithm::set_intersection(std::forward<L>(lhs), std::forward<R>(rhs), std::inserter(result, result.end()));
    return result;
}

}  // namespace cpp_pipelines::set