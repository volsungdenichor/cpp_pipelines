#pragma once

#include <algorithm>
#include <cpp_pipelines/functions.hpp>
#include <cpp_pipelines/pipeline.hpp>
#include <cpp_pipelines/subrange.hpp>
#include <cpp_pipelines/type_traits.hpp>
#include <functional>
#include <numeric>

namespace cpp_pipelines::algorithm
{
#define CPP_PIPELINES_CHECK_CONSTRAINTS(var, constraint) \
    static_assert(constraint<decltype(var)>::value, "'" #var "' - " #constraint " required")

struct return_found
{
    template <class Iter>
    constexpr auto operator()(Iter found, Iter, Iter) const -> Iter
    {
        return found;
    }
};

struct return_found_end
{
    template <class Iter>
    constexpr auto operator()(Iter found, Iter begin, Iter end) const -> subrange<Iter>
    {
        return subrange<Iter>{ found, end };
    }
};

struct return_begin_found
{
    template <class Iter>
    constexpr auto operator()(Iter found, Iter begin, Iter end) const -> subrange<Iter>
    {
        return subrange<Iter>{ begin, found };
    }
};

struct return_found_next
{
    template <class Iter>
    constexpr auto operator()(Iter found, Iter begin, Iter end) const -> subrange<Iter>
    {
        return found != end ? subrange<Iter>{ found, std::next(found) } : subrange<Iter>{ found, found };
    }
};

struct return_begin_next
{
    template <class Iter>
    constexpr auto operator()(Iter found, Iter begin, Iter end) const -> subrange<Iter>
    {
        return found != end ? subrange<Iter>{ begin, std::next(found) } : subrange<Iter>{ found, found };
    }
};

struct return_next_end
{
    template <class Iter>
    constexpr auto operator()(Iter found, Iter begin, Iter end) const -> subrange<Iter>
    {
        return found != end ? subrange<Iter>{ std::next(found), end } : subrange<Iter>{ found, found };
    }
};

struct return_ref
{
    template <class Iter>
    constexpr auto operator()(Iter found, Iter begin, Iter end) const -> iter_reference_t<Iter>
    {
        if (found == end)
            throw std::runtime_error{ "invalid iterator" };
        return *found;
    }
};

struct return_opt_found
{
    template <class Iter>
    constexpr auto operator()(Iter found, Iter begin, Iter end) const -> std::optional<Iter>
    {
        using result_type = std::optional<Iter>;
        return found != end ? result_type{ found } : result_type{};
    }
};

struct return_both
{
    template <class Iter>
    constexpr auto operator()(Iter found, Iter begin, Iter end) const
        -> std::pair<subrange<Iter>, subrange<Iter>>
    {
        return { subrange<Iter>{ begin, found }, subrange<Iter>{ found, end } };
    }
};

using default_return_policy = return_found;

namespace detail
{
template <class T>
constexpr auto equal_to(T&& value)
{
    return std::bind(std::equal_to{}, std::placeholders::_1, std::forward<T>(value));
}

template <class Func, class Proj1, class Proj2>
struct invoke_binary
{
    Func func;
    Proj1 proj1;
    Proj2 proj2;

    template <class T, class U>
    constexpr decltype(auto) operator()(T&& lhs, U&& rhs) const
    {
        return invoke(func, invoke(proj1, std::forward<T>(lhs)), invoke(proj2, std::forward<U>(rhs)));
    }
};

template <class Func, class Proj>
struct invoke_binary<Func, Proj, void>
{
    Func func;
    Proj proj;

    template <class T, class U>
    constexpr decltype(auto) operator()(T&& lhs, U&& rhs) const
    {
        return invoke(func, invoke(proj, std::forward<T>(lhs)), invoke(proj, std::forward<U>(rhs)));
    }
};

template <class Func, class Proj>
invoke_binary(Func, Proj) -> invoke_binary<Func, Proj, void>;

template <class Func, class Proj1, class Proj2>
invoke_binary(Func, Proj1, Proj2) -> invoke_binary<Func, Proj1, Proj2>;

template <class Output, class T>
void yield(Output& output, T&& item)
{
    *output = std::forward<T>(item);
    ++output;
}

template <class Policy, class Iter, class Func>
decltype(auto) invoke_algorithm(Iter b, Iter e, Func&& func)
{
    static const auto policy = Policy{};
    auto it = std::invoke(std::forward<Func>(func), b, e);
    return policy(it, b, e);
}

template <class Iter, class OutputIter, class BinaryFunc, class Proj>
auto adjacent_difference(Iter b, Iter e, OutputIter output, BinaryFunc func, Proj proj)
{
    if (b == e)
        return output;

    auto prev = invoke(proj, *b);
    yield(output, prev);
    while (++b != e)
    {
        auto cur = invoke(proj, *b);
        yield(output, invoke(func, cur, std::move(prev)));
        prev = std::move(cur);
    }
    return output;
}

template <class Iter, class Op>
Iter get_bound(Iter b, Iter e, Op op)
{
    Iter it;
    auto count = std::distance(b, e);
    auto step = count / 2;

    while (count > 0)
    {
        it = b;
        step = count / 2;
        std::advance(it, step);
        if (op(it))
        {
            b = ++it;
            count -= step + 1;
        }
        else
            count = step;
    }
    return b;
}

template <class Iter, class T, class Compare, class Proj>
Iter lower_bound(Iter b, Iter e, const T& value, Compare compare, Proj proj)
{
    return get_bound(b, e, [&](auto it) { return invoke(compare, invoke(proj, *it), value); });
}

template <class Iter, class T, class Compare, class Proj>
Iter upper_bound(Iter b, Iter e, const T& value, Compare compare, Proj proj)
{
    return get_bound(b, e, [&](auto it) { return !invoke(compare, value, invoke(proj, *it)); });
}

template <class Iter, class T, class Compare, class Proj>
auto equal_range(Iter b, Iter e, const T& value, Compare compare, Proj proj) -> subrange<Iter>
{
    return { lower_bound(b, e, value, std::ref(compare), std::ref(proj)),
             upper_bound(b, e, value, std::ref(compare), std::ref(proj)) };
}

template <class Iter, class Output, class BinaryFunc, class Proj>
Output partial_sum(Iter b, Iter e, Output output, BinaryFunc func, Proj proj)
{
    if (b == e)
        return output;

    auto sum = invoke(proj, *b);
    yield(output, sum);

    while (++b != e)
    {
        sum = invoke(func, std::move(sum), invoke(proj, *b));
        yield(output, sum);
    }
    return output;
}

template <class Iter1, class Iter2, class Func>
Iter1 overwrite(Iter1 src_b, Iter1 src_e, Iter2 dst_b, Iter2 dst_e, Func func)
{
    for (; src_b != src_e && dst_b != dst_e; ++src_b, ++dst_b)
    {
        *dst_b = invoke(func, *src_b);
    }
    return src_b;
}

}  // namespace detail

template <class Range, class T, class BinaryFunc = std::plus<>, class Proj = identity_fn>
auto accumulate(Range&& range, T init, BinaryFunc func = {}, Proj proj = {})
{
    CPP_PIPELINES_CHECK_CONSTRAINTS(range, is_input_range);

    return std::accumulate(std::begin(range), std::end(range), init, [&](T total, auto&& item) {
        return invoke(func, std::move(total), invoke(proj, std::forward<decltype(item)>(item)));
    });
}

template <class Range, class OutputIter, class BinaryFunc = std::minus<>, class Proj = identity_fn>
auto adjacent_difference(Range&& range, OutputIter output, BinaryFunc func = {}, Proj proj = {})
{
    CPP_PIPELINES_CHECK_CONSTRAINTS(range, is_input_range);

    return detail::adjacent_difference(std::begin(range), std::end(range), output, std::ref(func), std::ref(proj));
}

template <class Range, class UnaryPred, class Proj = identity_fn>
auto all_of(Range&& range, UnaryPred pred, Proj proj = {})
{
    CPP_PIPELINES_CHECK_CONSTRAINTS(range, is_input_range);

    return std::all_of(std::begin(range), std::end(range), fn(std::ref(proj), std::ref(pred)));
}

template <class Range, class UnaryPred, class Proj = identity_fn>
auto any_of(Range&& range, UnaryPred pred, Proj proj = {})
{
    CPP_PIPELINES_CHECK_CONSTRAINTS(range, is_input_range);

    return std::any_of(std::begin(range), std::end(range), fn(std::ref(proj), std::ref(pred)));
}

template <class Range, class OutputIter>
auto copy(Range&& range, OutputIter output)
{
    CPP_PIPELINES_CHECK_CONSTRAINTS(range, is_input_range);

    return std::copy(std::begin(range), std::end(range), output);
}

template <class Range, class OutputIter, class UnaryPred, class Proj = identity_fn>
auto copy_if(Range&& range, OutputIter output, UnaryPred pred, Proj proj = {})
{
    CPP_PIPELINES_CHECK_CONSTRAINTS(range, is_input_range);

    return std::copy_if(std::begin(range), std::end(range), output, fn(std::ref(proj), std::ref(pred)));
}

template <class Range, class Size, class OutputIter>
auto copy_n(Range&& range, Size size, OutputIter output)
{
    CPP_PIPELINES_CHECK_CONSTRAINTS(range, is_input_range);

    return std::copy_n(std::begin(range), size, output);
}

template <class Range, class T, class Proj = identity_fn>
auto count(Range&& range, const T& value, Proj proj = {})
{
    CPP_PIPELINES_CHECK_CONSTRAINTS(range, is_input_range);

    return std::count_if(std::begin(range), std::end(range), fn(std::ref(proj), detail::equal_to(std::ref(value))));
}

template <class Range, class UnaryPred, class Proj = identity_fn>
auto count_if(Range&& range, UnaryPred pred, Proj proj = {})
{
    CPP_PIPELINES_CHECK_CONSTRAINTS(range, is_input_range);

    return std::count_if(std::begin(range), std::end(range), fn(std::ref(proj), std::ref(pred)));
}

template <class Range1, class Range2, class BinaryPred = std::equal_to<>, class Proj1 = identity_fn, class Proj2 = identity_fn>
auto equal(Range1&& range1, Range2&& range2, BinaryPred pred = {}, Proj1 proj1 = {}, Proj2 proj2 = {})
{
    CPP_PIPELINES_CHECK_CONSTRAINTS(range1, is_input_range);
    CPP_PIPELINES_CHECK_CONSTRAINTS(range2, is_input_range);

    return std::equal(
        std::begin(range1),
        std::end(range1),
        std::begin(range2),
        std::end(range2),
        detail::invoke_binary{ std::ref(pred), std::ref(proj1), std::ref(proj2) });
}

template <class Range, class T, class Compare = std::less<>, class Proj = identity_fn>
auto equal_range(Range&& range, const T& value, Compare compare = {}, Proj proj = {})
{
    CPP_PIPELINES_CHECK_CONSTRAINTS(range, is_forward_range);

    return detail::equal_range(std::begin(range), std::end(range), value, std::ref(compare), std::ref(proj));
}

template <class Range, class Output, class T, class BinaryFunc, class Proj = identity_fn>
auto exclusive_scan(Range&& range, Output output, T init, BinaryFunc func, Proj proj = {})
{
    return std::transform_exclusive_scan(std::begin(range), std::end(range), output, init, std::ref(func), std::ref(proj));
}

template <class Range, class T>
void fill(Range&& range, const T& value)
{
    CPP_PIPELINES_CHECK_CONSTRAINTS(range, is_forward_range);

    std::fill(std::begin(range), std::end(range), value);
}

template <class Policy = default_return_policy, class Range, class T, class Proj = identity_fn>
decltype(auto) find(Range&& range, const T& value, Proj proj = {})
{
    CPP_PIPELINES_CHECK_CONSTRAINTS(range, is_input_range);

    return detail::invoke_algorithm<Policy>(std::begin(range), std::end(range), [&](auto b, auto e) {
        return std::find_if(b, e, fn(std::ref(proj), detail::equal_to(std::ref(value))));
    });
}

template <class Policy = default_return_policy, class Range, class UnaryPred, class Proj = identity_fn>
decltype(auto) find_if(Range&& range, UnaryPred pred, Proj proj = {})
{
    CPP_PIPELINES_CHECK_CONSTRAINTS(range, is_input_range);

    return detail::invoke_algorithm<Policy>(std::begin(range), std::end(range), [&](auto b, auto e) {
        return std::find_if(b, e, fn(std::ref(proj), std::ref(pred)));
    });
}

template <class Policy = default_return_policy, class Range, class UnaryPred, class Proj = identity_fn>
decltype(auto) find_if_not(Range&& range, UnaryPred pred, Proj proj = {})
{
    CPP_PIPELINES_CHECK_CONSTRAINTS(range, is_input_range);

    return detail::invoke_algorithm<Policy>(std::begin(range), std::end(range), [&](auto b, auto e) {
        return std::find_if_not(b, e, fn(std::ref(proj), std::ref(pred)));
    });
}

template <
    class Policy = default_return_policy,
    class Range1,
    class Range2,
    class BinaryPred = std::equal_to<>,
    class Proj1 = identity_fn,
    class Proj2 = identity_fn>
decltype(auto) find_end(Range1&& range1, Range2&& range2, BinaryPred pred = {}, Proj1 proj1 = {}, Proj2 proj2 = {})
{
    CPP_PIPELINES_CHECK_CONSTRAINTS(range1, is_forward_range);
    CPP_PIPELINES_CHECK_CONSTRAINTS(range2, is_forward_range);

    return detail::invoke_algorithm<Policy>(std::begin(range1), std::end(range1), [&](auto b, auto e) {
        return std::find_end(
            b,
            e,
            std::begin(range2),
            std::end(range2),
            detail::invoke_binary{ std::ref(pred), std::ref(proj1), std::ref(proj2) });
    });
}

template <
    class Policy = default_return_policy,
    class Range1,
    class Range2,
    class BinaryPred = std::equal_to<>,
    class Proj1 = identity_fn,
    class Proj2 = identity_fn>
decltype(auto) find_first_of(Range1&& range1, Range2&& range2, BinaryPred pred = {}, Proj1 proj1 = {}, Proj2 proj2 = {})
{
    CPP_PIPELINES_CHECK_CONSTRAINTS(range1, is_forward_range);
    CPP_PIPELINES_CHECK_CONSTRAINTS(range2, is_forward_range);

    return detail::invoke_algorithm<Policy>(std::begin(range1), std::end(range1), [&](auto b, auto e) {
        return std::find_first_of(
            b,
            e,
            std::begin(range2),
            std::end(range2),
            detail::invoke_binary{ std::ref(pred), std::ref(proj1), std::ref(proj2) });
    });
}

template <class Range, class UnaryFunc, class Proj = identity_fn>
auto for_each(Range&& range, UnaryFunc func, Proj proj = {})
{
    CPP_PIPELINES_CHECK_CONSTRAINTS(range, is_input_range);

    return std::for_each(std::begin(range), std::end(range), fn(std::ref(proj), std::ref(func)));
}

template <class Range, class Generator>
void generate(Range&& range, Generator generator)
{
    CPP_PIPELINES_CHECK_CONSTRAINTS(range, is_forward_range);

    std::generate(std::begin(range), std::end(range), std::move(generator));
}

template <class OutputIter, class Size, class Generator>
void generate_n(OutputIter output, Size size, Generator generator)
{
    std::generate_n(output, size, std::move(generator));
}

template <class Range1, class Range2, class Compare = std::less<>, class Proj1 = identity_fn, class Proj2 = identity_fn>
auto includes(Range1&& range1, Range2&& range2, Compare compare = {}, Proj1 proj1 = {}, Proj2 proj2 = {})
{
    CPP_PIPELINES_CHECK_CONSTRAINTS(range1, is_input_range);
    CPP_PIPELINES_CHECK_CONSTRAINTS(range2, is_input_range);

    return std::includes(
        std::begin(range1),
        std::end(range1),
        std::begin(range2),
        std::end(range2),
        detail::invoke_binary{ std::ref(compare), std::ref(proj1), std::ref(proj2) });
}

template <class Range, class Output, class BinaryFunc, class Proj = identity_fn>
auto inclusive_scan(Range&& range, Output output, BinaryFunc func, Proj proj = {})
{
    return std::transform_inclusive_scan(std::begin(range), std::end(range), output, std::ref(func), std::ref(proj));
}

template <
    class Range1,
    class Range2,
    class T,
    class BinaryFunc1 = std::plus<>,
    class BinaryFunc2 = std::multiplies<>,
    class Proj1 = identity_fn,
    class Proj2 = identity_fn>
auto inner_product(
    Range1&& range1,
    Range2&& range2,
    T init,
    BinaryFunc1 func1 = {},
    BinaryFunc2 func2 = {},
    Proj1 proj1 = {},
    Proj2 proj2 = {})
{
    CPP_PIPELINES_CHECK_CONSTRAINTS(range1, is_input_range);
    CPP_PIPELINES_CHECK_CONSTRAINTS(range2, is_input_range);

    return std::inner_product(
        std::begin(range1),
        std::end(range1),
        std::begin(range2),
        init,
        std::ref(func1),
        detail::invoke_binary{ std::ref(func2), std::ref(proj1), std::ref(proj2) });
}

template <class Range, class T>
void iota(Range&& range, T value)
{
    CPP_PIPELINES_CHECK_CONSTRAINTS(range, is_forward_range);

    std::iota(std::begin(range), std::end(range), std::move(value));
}

template <class Range, class Compare = std::less<>, class Proj = identity_fn>
auto is_heap(Range&& range, Compare compare = {}, Proj proj = {})
{
    CPP_PIPELINES_CHECK_CONSTRAINTS(range, is_random_access_range);

    return std::is_heap(std::begin(range), std::end(range), detail::invoke_binary{ std::ref(compare), std::ref(proj) });
}

template <class Policy = default_return_policy, class Range, class Compare = std::less<>, class Proj = identity_fn>
decltype(auto) is_heap_until(Range&& range, Compare compare = {}, Proj proj = {})
{
    CPP_PIPELINES_CHECK_CONSTRAINTS(range, is_random_access_range);

    return detail::invoke_algorithm<Policy>(std::begin(range), std::end(range), [&](auto b, auto e) {
        return std::is_heap_until(b, e, detail::invoke_binary{ std::ref(compare), std::ref(proj) });
    });
}

template <class Range, class UnaryPred, class Proj = identity_fn>
auto is_partitioned(Range&& range, UnaryPred pred, Proj proj = {})
{
    CPP_PIPELINES_CHECK_CONSTRAINTS(range, is_input_range);

    return std::is_partitioned(std::begin(range), std::end(range), fn(std::ref(proj), std::ref(pred)));
}

template <class Range1, class Range2, class BinaryPred = std::equal_to<>, class Proj1 = identity_fn, class Proj2 = identity_fn>
auto is_permutation(Range1&& range1, Range2&& range2, BinaryPred pred = {}, Proj1 proj1 = {}, Proj2 proj2 = {})
{
    CPP_PIPELINES_CHECK_CONSTRAINTS(range1, is_forward_range);
    CPP_PIPELINES_CHECK_CONSTRAINTS(range2, is_forward_range);

    return std::is_permutation(
        std::begin(range1),
        std::end(range1),
        std::begin(range2),
        detail::invoke_binary{ std::ref(pred), std::ref(proj1), std::ref(proj2) });
}

template <class Range, class Compare = std::less<>, class Proj = identity_fn>
auto is_sorted(Range&& range, Compare compare = {}, Proj proj = {})
{
    CPP_PIPELINES_CHECK_CONSTRAINTS(range, is_forward_range);

    return std::is_sorted(std::begin(range), std::end(range), detail::invoke_binary{ std::ref(compare), std::ref(proj) });
}

template <class Policy = default_return_policy, class Range, class Compare = std::less<>, class Proj = identity_fn>
decltype(auto) is_sorted_until(Range&& range, Compare compare = {}, Proj proj = {})
{
    CPP_PIPELINES_CHECK_CONSTRAINTS(range, is_forward_range);

    return detail::invoke_algorithm<Policy>(std::begin(range), std::end(range), [&](auto b, auto e) {
        return std::is_sorted_until(b, e, detail::invoke_binary{ std::ref(compare), std::ref(proj) });
    });
}

template <class Range1, class Range2, class Compare = std::less<>, class Proj1 = identity_fn, class Proj2 = identity_fn>
auto lexicographical_compare(Range1&& range1, Range2&& range2, Compare compare = {}, Proj1 proj1 = {}, Proj2 proj2 = {})
{
    CPP_PIPELINES_CHECK_CONSTRAINTS(range1, is_input_range);
    CPP_PIPELINES_CHECK_CONSTRAINTS(range2, is_input_range);

    return std::lexicographical_compare(
        std::begin(range1),
        std::end(range1),
        std::begin(range2),
        std::end(range2),
        detail::invoke_binary{ std::ref(compare), std::ref(proj1), std::ref(proj2) });
}

template <class Policy = default_return_policy, class Range, class T, class Compare = std::less<>, class Proj = identity_fn>
decltype(auto) lower_bound(Range&& range, const T& value, Compare compare = {}, Proj proj = {})
{
    CPP_PIPELINES_CHECK_CONSTRAINTS(range, is_forward_range);

    return detail::invoke_algorithm<Policy>(std::begin(range), std::end(range), [&](auto b, auto e) {
        return detail::lower_bound(b, e, value, std::ref(compare), std::ref(proj));
    });
}

template <class Range, class Compare = std::less<>, class Proj = identity_fn>
void make_heap(Range&& range, Compare compare = {}, Proj proj = {})
{
    CPP_PIPELINES_CHECK_CONSTRAINTS(range, is_random_access_range);

    std::make_heap(std::begin(range), std::end(range), detail::invoke_binary{ std::ref(compare), std::ref(proj) });
}

template <class Policy = default_return_policy, class Range, class Compare = std::less<>, class Proj = identity_fn>
decltype(auto) max_element(Range&& range, Compare compare = {}, Proj proj = {})
{
    CPP_PIPELINES_CHECK_CONSTRAINTS(range, is_forward_range);

    return detail::invoke_algorithm<Policy>(std::begin(range), std::end(range), [&](auto b, auto e) {
        return std::max_element(b, e, detail::invoke_binary{ std::ref(compare), std::ref(proj) });
    });
}

template <
    class Range1,
    class Range2,
    class OutputIter,
    class Compare = std::less<>,
    class Proj1 = identity_fn,
    class Proj2 = identity_fn>
auto merge(Range1&& range1, Range2&& range2, OutputIter output, Compare compare = {}, Proj1 proj1 = {}, Proj2 proj2 = {})
{
    CPP_PIPELINES_CHECK_CONSTRAINTS(range1, is_input_range);
    CPP_PIPELINES_CHECK_CONSTRAINTS(range2, is_input_range);

    return std::merge(
        std::begin(range1),
        std::end(range1),
        std::begin(range2),
        std::end(range2),
        output,
        detail::invoke_binary{ std::ref(compare), std::ref(proj1), std::ref(proj2) });
}

template <class Range, class Compare = std::less<>, class Proj = identity_fn>
void inplace_merge(Range&& range, iterator_t<Range> middle, Compare compare = {}, Proj proj = {})
{
    CPP_PIPELINES_CHECK_CONSTRAINTS(range, is_bidirectional_range);

    std::inplace_merge(
        std::begin(range), middle, std::end(range), detail::invoke_binary{ std::ref(compare), std::ref(proj) });
}

template <class Policy = default_return_policy, class Range, class Compare = std::less<>, class Proj = identity_fn>
auto minmax_element(Range&& range, Compare compare = {}, Proj proj = {})
{
    CPP_PIPELINES_CHECK_CONSTRAINTS(range, is_forward_range);

    static const auto policy = Policy{};
    auto [b, e] = make_range(range);
    auto [min, max] = std::minmax_element(b, e, detail::invoke_binary{ std::ref(compare), std::ref(proj) });
    return std::forward_as_tuple(policy(min, b, e), policy(max, b, e));
}

template <class Policy = default_return_policy, class Range, class Compare = std::less<>, class Proj = identity_fn>
decltype(auto) min_element(Range&& range, Compare compare = {}, Proj proj = {})
{
    CPP_PIPELINES_CHECK_CONSTRAINTS(range, is_forward_range);

    return detail::invoke_algorithm<Policy>(std::begin(range), std::end(range), [&](auto b, auto e) {
        return std::min_element(b, e, detail::invoke_binary{ std::ref(compare), std::ref(proj) });
    });
}

template <
    class Policy = default_return_policy,
    class Range1,
    class Range2,
    class BinaryPred = std::equal_to<>,
    class Proj1 = identity_fn,
    class Proj2 = identity_fn>
decltype(auto) mismatch(Range1&& range1, Range2&& range2, BinaryPred pred = {}, Proj1 proj1 = {}, Proj2 proj2 = {})
{
    CPP_PIPELINES_CHECK_CONSTRAINTS(range1, is_input_range);
    CPP_PIPELINES_CHECK_CONSTRAINTS(range2, is_input_range);

    static const auto policy = Policy{};
    auto [b1, e1] = make_range(range1);
    auto [b2, e2] = make_range(range2);
    auto [b, e] = std::mismatch(b1, e1, b2, detail::invoke_binary{ std::ref(pred), std::ref(proj1), std::ref(proj2) });
    return std::forward_as_tuple(policy(b, b1, e1), policy(e, b2, e2));
}

template <class Range, class OutputIter>
auto move(Range&& range, OutputIter output)
{
    CPP_PIPELINES_CHECK_CONSTRAINTS(range, is_input_range);

    return std::move(std::begin(range), std::end(range), output);
}

template <class Range, class Compare = std::less<>, class Proj = identity_fn>
auto next_permutation(Range&& range, Compare compare = {}, Proj proj = {})
{
    CPP_PIPELINES_CHECK_CONSTRAINTS(range, is_bidirectional_range);

    return std::next_permutation(
        std::begin(range), std::end(range), detail::invoke_binary{ std::ref(compare), std::ref(proj) });
}

template <class Range, class UnaryPred, class Proj = identity_fn>
auto none_of(Range&& range, UnaryPred pred, Proj proj = {})
{
    CPP_PIPELINES_CHECK_CONSTRAINTS(range, is_input_range);

    return std::none_of(std::begin(range), std::end(range), fn(std::ref(proj), std::ref(pred)));
}

template <class Range, class Compare = std::less<>, class Proj = identity_fn>
void nth_element(Range&& range, iterator_t<Range> middle, Compare compare = {}, Proj proj = {})
{
    CPP_PIPELINES_CHECK_CONSTRAINTS(range, is_random_access_range);

    std::nth_element(std::begin(range), middle, std::end(range), detail::invoke_binary{ std::ref(compare), std::ref(proj) });
}

template <class Range, class Compare = std::less<>, class Proj = identity_fn>
void partial_sort(Range&& range, iterator_t<Range> middle, Compare compare = {}, Proj proj = {})
{
    CPP_PIPELINES_CHECK_CONSTRAINTS(range, is_random_access_range);

    std::partial_sort(
        std::begin(range), middle, std::end(range), detail::invoke_binary{ std::ref(compare), std::ref(proj) });
}
template <
    class Policy = default_return_policy,
    class Range1,
    class Range2,
    class Compare = std::less<>,
    class Proj1 = identity_fn,
    class Proj2 = identity_fn>
decltype(auto) partial_sort_copy(Range1&& range1, Range2&& range2, Compare compare = {}, Proj1 proj1 = {}, Proj2 proj2 = {})
{
    CPP_PIPELINES_CHECK_CONSTRAINTS(range1, is_input_range);
    CPP_PIPELINES_CHECK_CONSTRAINTS(range2, is_input_range);

    return detail::invoke_algorithm<Policy>(std::begin(range2), std::end(range2), [&](auto b, auto e) {
        return std::partial_sort_copy(
            std::begin(range1),
            std::end(range1),
            b,
            e,
            detail::invoke_binary{ std::ref(compare), std::ref(proj1), std::ref(proj2) });
    });
}

template <class Range, class OutputIter, class BinaryFunc = std::plus<>, class Proj = identity_fn>
auto partial_sum(Range&& range, OutputIter output, BinaryFunc func = {}, Proj proj = {})
{
    CPP_PIPELINES_CHECK_CONSTRAINTS(range, is_input_range);

    return detail::partial_sum(std::begin(range), std::end(range), output, std::ref(func), std::ref(proj));
}

template <class Policy = default_return_policy, class Range, class UnaryPred, class Proj = identity_fn>
decltype(auto) partition(Range&& range, UnaryPred pred, Proj proj = {})
{
    CPP_PIPELINES_CHECK_CONSTRAINTS(range, is_forward_range);

    return detail::invoke_algorithm<Policy>(std::begin(range), std::end(range), [&](auto b, auto e) {
        return std::partition(b, e, fn(std::ref(proj), std::ref(pred)));
    });
}

template <class Range, class OutputIter1, class OutputIter2, class UnaryPred, class Proj = identity_fn>
auto partition_copy(Range&& range, OutputIter1 result_true, OutputIter2 result_false, UnaryPred pred, Proj proj = {})
{
    CPP_PIPELINES_CHECK_CONSTRAINTS(range, is_input_range);

    return std::partition_copy(
        std::begin(range), std::end(range), result_true, result_false, fn(std::ref(proj), std::ref(pred)));
}

template <class Policy = default_return_policy, class Range, class UnaryPred, class Proj = identity_fn>
decltype(auto) stable_partition(Range&& range, UnaryPred pred, Proj proj = {})
{
    CPP_PIPELINES_CHECK_CONSTRAINTS(range, is_bidirectional_range);

    return detail::invoke_algorithm<Policy>(std::begin(range), std::end(range), [&](auto b, auto e) {
        return std::stable_partition(b, e, fn(std::ref(proj), std::ref(pred)));
    });
}

template <class Range, class Compare = std::less<>, class Proj = identity_fn>
auto prev_permutation(Range&& range, Compare compare = {}, Proj proj = {})
{
    CPP_PIPELINES_CHECK_CONSTRAINTS(range, is_bidirectional_range);

    return std::prev_permutation(
        std::begin(range), std::end(range), detail::invoke_binary{ std::ref(compare), std::ref(proj) });
}

template <class Range, class Compare = std::less<>, class Proj = identity_fn>
void push_heap(Range&& range, Compare compare = {}, Proj proj = {})
{
    CPP_PIPELINES_CHECK_CONSTRAINTS(range, is_random_access_range);

    std::push_heap(std::begin(range), std::end(range), detail::invoke_binary{ std::ref(compare), std::ref(proj) });
}

template <class Range, class T, class BinaryFunc = std::plus<>, class Proj = identity_fn>
auto reduce(Range&& range, T init, BinaryFunc func = {}, Proj proj = {})
{
    return std::transform_reduce(std::begin(range), std::end(range), std::move(init), std::ref(func), std::ref(proj));
}

template <class Policy = default_return_policy, class Range, class T, class Proj = identity_fn>
decltype(auto) remove(Range&& range, const T& value, Proj proj = {})
{
    CPP_PIPELINES_CHECK_CONSTRAINTS(range, is_forward_range);

    return detail::invoke_algorithm<Policy>(std::begin(range), std::end(range), [&](auto b, auto e) {
        return std::remove_if(b, e, fn(std::ref(proj), detail::equal_to(std::ref(value))));
    });
}

template <class Policy = default_return_policy, class Range, class UnaryPred, class Proj = identity_fn>
decltype(auto) remove_if(Range&& range, UnaryPred pred, Proj proj = {})
{
    CPP_PIPELINES_CHECK_CONSTRAINTS(range, is_forward_range);

    return detail::invoke_algorithm<Policy>(std::begin(range), std::end(range), [&](auto b, auto e) {
        return std::remove_if(b, e, fn(std::ref(proj), std::ref(pred)));
    });
}

template <class Range, class OutputIter, class T, class Proj = identity_fn>
auto remove_copy(Range&& range, OutputIter output, const T& value, Proj proj = {})
{
    CPP_PIPELINES_CHECK_CONSTRAINTS(range, is_input_range);

    return std::remove_copy_if(std::begin(range), std::end(range), output, fn(std::ref(proj), detail::equal_to(std::ref(value))));
}

template <class Range, class OutputIter, class UnaryPred, class Proj = identity_fn>
auto remove_copy_if(Range&& range, OutputIter output, UnaryPred pred, Proj proj = {})
{
    CPP_PIPELINES_CHECK_CONSTRAINTS(range, is_input_range);

    return std::remove_copy_if(std::begin(range), std::end(range), output, fn(std::ref(proj), std::ref(pred)));
}

template <class Range, class T1, class T2, class Proj = identity_fn>
void raplace(Range&& range, const T1& old_value, const T2& new_value, Proj proj = {})
{
    CPP_PIPELINES_CHECK_CONSTRAINTS(range, is_forward_range);

    std::replace_if(std::begin(range), std::end(range), fn(std::ref(proj), detail::equal_to(std::ref(old_value))), new_value);
}

template <class Range, class UnaryPred, class T, class Proj = identity_fn>
void raplace_if(Range&& range, UnaryPred pred, const T& new_value, Proj proj = {})
{
    CPP_PIPELINES_CHECK_CONSTRAINTS(range, is_forward_range);

    std::replace_if(std::begin(range), std::end(range), fn(std::ref(proj), std::ref(pred)), new_value);
}

template <class Range, class OutputIter, class T1, class T2, class Proj = identity_fn>
auto replace_copy(Range&& range, OutputIter output, const T1& old_value, const T2& new_value, Proj proj = {})
{
    CPP_PIPELINES_CHECK_CONSTRAINTS(range, is_input_range);

    return std::replace_copy_if(
        std::begin(range), std::end(range), output, fn(std::ref(proj), detail::equal_to(std::ref(old_value))), new_value);
}

template <class Range, class OutputIter, class UnaryPred, class T, class Proj = identity_fn>
auto replace_copy_if(Range&& range, OutputIter output, UnaryPred pred, const T& new_value, Proj proj = {})
{
    CPP_PIPELINES_CHECK_CONSTRAINTS(range, is_input_range);

    return std::replace_copy_if(std::begin(range), std::end(range), output, fn(std::ref(proj), std::ref(pred)), new_value);
}

template <class Range>
void reverse(Range&& range)
{
    CPP_PIPELINES_CHECK_CONSTRAINTS(range, is_bidirectional_range);

    std::reverse(std::begin(range), std::end(range));
}

template <class Range, class OutputIter>
auto reverse_copy(Range&& range, OutputIter output)
{
    CPP_PIPELINES_CHECK_CONSTRAINTS(range, is_bidirectional_range);

    return std::reverse_copy(std::begin(range), std::end(range), output);
}

template <class Policy = default_return_policy, class Range>
decltype(auto) rotate(Range&& range, iterator_t<Range> middle)
{
    CPP_PIPELINES_CHECK_CONSTRAINTS(range, is_forward_range);

    return detail::invoke_algorithm<Policy>(
        std::begin(range), std::end(range), [&](auto b, auto e) { return std::rotate(b, middle, e); });
}

template <class Range, class OutputIter>
auto rotate_copy(Range&& range, iterator_t<Range> middle, OutputIter output)
{
    CPP_PIPELINES_CHECK_CONSTRAINTS(range, is_forward_range);

    return std::rotate_copy(std::begin(range), middle, std::end(range), output);
}

template <
    class Policy = default_return_policy,
    class Range1,
    class Range2,
    class BinaryPred = std::equal_to<>,
    class Proj1 = identity_fn,
    class Proj2 = identity_fn,
    class = std::enable_if_t<is_forward_range<Range2>::value>>
decltype(auto) search(Range1&& range1, Range2&& range2, BinaryPred pred = {}, Proj1 proj1 = {}, Proj2 proj2 = {})
{
    CPP_PIPELINES_CHECK_CONSTRAINTS(range1, is_forward_range);
    CPP_PIPELINES_CHECK_CONSTRAINTS(range2, is_forward_range);

    return detail::invoke_algorithm<Policy>(std::begin(range1), std::end(range1), [&](auto b, auto e) {
        return std::search(
            b,
            e,
            std::begin(range2),
            std::end(range2),
            detail::invoke_binary{ std::ref(pred), std::ref(proj1), std::ref(proj2) });
    });
}

template <class Range, class BinaryPred = std::equal_to<>>
auto default_searcher(Range&& range, BinaryPred pred = {})
{
    CPP_PIPELINES_CHECK_CONSTRAINTS(range, is_forward_range);

    return std::default_searcher{ std::begin(range), std::end(range), std::move(pred) };
}

template <class Range, class Hash = std::hash<range_value_t<Range>>, class BinaryPred = std::equal_to<>>
auto boyer_moore_searcher(Range&& range, Hash hash = {}, BinaryPred pred = {})
{
    CPP_PIPELINES_CHECK_CONSTRAINTS(range, is_random_access_range);

    return std::boyer_moore_searcher{ std::begin(range), std::end(range), std::move(hash), std::move(pred) };
}

template <class Range, class Hash = std::hash<range_value_t<Range>>, class BinaryPred = std::equal_to<>>
auto boyer_moore_horspool_searcher(Range&& range, Hash hash = {}, BinaryPred pred = {})
{
    CPP_PIPELINES_CHECK_CONSTRAINTS(range, is_random_access_range);

    return std::boyer_moore_horspool_searcher{ std::begin(range), std::end(range), std::move(hash), std::move(pred) };
}

template <
    class Policy = default_return_policy,
    class Range,
    class Searcher,
    class = decltype(std::declval<Searcher>()(std::declval<iterator_t<Range>>(), std::declval<iterator_t<Range>>()))>
decltype(auto) search(Range&& range, const Searcher& searcher)
{
    CPP_PIPELINES_CHECK_CONSTRAINTS(range, is_forward_range);

    return detail::invoke_algorithm<Policy>(
        std::begin(range), std::end(range), [&](auto b, auto e) { return std::search(b, e, searcher); });
}

template <
    class Policy = default_return_policy,
    class Range,
    class Size,
    class T,
    class BinaryPred = std::equal_to<>,
    class Proj = identity_fn>
decltype(auto) search_n(Range&& range, Size size, const T& value, BinaryPred pred = {}, Proj proj = {})
{
    CPP_PIPELINES_CHECK_CONSTRAINTS(range, is_forward_range);

    return detail::invoke_algorithm<Policy>(std::begin(range), std::end(range), [&](auto b, auto e) {
        return std::search_n(b, e, size, value, detail::invoke_binary{ std::ref(pred), std::ref(proj) });
    });
}

template <
    class Range1,
    class Range2,
    class OutputIter,
    class Compare = std::less<>,
    class Proj1 = identity_fn,
    class Proj2 = identity_fn>
auto set_difference(
    Range1&& range1, Range2&& range2, OutputIter output, Compare compare = {}, Proj1 proj1 = {}, Proj2 proj2 = {})
{
    CPP_PIPELINES_CHECK_CONSTRAINTS(range1, is_input_range);
    CPP_PIPELINES_CHECK_CONSTRAINTS(range2, is_input_range);

    return std::set_difference(
        std::begin(range1),
        std::end(range1),
        std::begin(range2),
        std::end(range2),
        output,
        detail::invoke_binary{ std::ref(compare), std::ref(proj1), std::ref(proj2) });
}

template <
    class Range1,
    class Range2,
    class OutputIter,
    class Compare = std::less<>,
    class Proj1 = identity_fn,
    class Proj2 = identity_fn>
auto set_intersection(
    Range1&& range1, Range2&& range2, OutputIter output, Compare compare = {}, Proj1 proj1 = {}, Proj2 proj2 = {})
{
    CPP_PIPELINES_CHECK_CONSTRAINTS(range1, is_input_range);
    CPP_PIPELINES_CHECK_CONSTRAINTS(range2, is_input_range);

    return std::set_intersection(
        std::begin(range1),
        std::end(range1),
        std::begin(range2),
        std::end(range2),
        output,
        detail::invoke_binary{ std::ref(compare), std::ref(proj1), std::ref(proj2) });
}

template <
    class Range1,
    class Range2,
    class OutputIter,
    class Compare = std::less<>,
    class Proj1 = identity_fn,
    class Proj2 = identity_fn>
auto set_symmetric_difference(
    Range1&& range1, Range2&& range2, OutputIter output, Compare compare = {}, Proj1 proj1 = {}, Proj2 proj2 = {})
{
    CPP_PIPELINES_CHECK_CONSTRAINTS(range1, is_input_range);
    CPP_PIPELINES_CHECK_CONSTRAINTS(range2, is_input_range);

    return std::set_symmetric_difference(
        std::begin(range1),
        std::end(range1),
        std::begin(range2),
        std::end(range2),
        output,
        detail::invoke_binary{ std::ref(compare), std::ref(proj1), std::ref(proj2) });
}

template <
    class Range1,
    class Range2,
    class OutputIter,
    class Compare = std::less<>,
    class Proj1 = identity_fn,
    class Proj2 = identity_fn>
auto set_union(Range1&& range1, Range2&& range2, OutputIter output, Compare compare = {}, Proj1 proj1 = {}, Proj2 proj2 = {})
{
    CPP_PIPELINES_CHECK_CONSTRAINTS(range1, is_input_range);
    CPP_PIPELINES_CHECK_CONSTRAINTS(range2, is_input_range);

    return std::set_union(
        std::begin(range1),
        std::end(range1),
        std::begin(range2),
        std::end(range2),
        output,
        detail::invoke_binary{ std::ref(compare), std::ref(proj1), std::ref(proj2) });
}

template <class Range, class RandomNumberGenerator>
void shuffle(Range&& range, RandomNumberGenerator generator)
{
    CPP_PIPELINES_CHECK_CONSTRAINTS(range, is_random_access_range);

    std::shuffle(std::begin(range), std::end(range), std::move(generator));
}

template <class Range, class Compare = std::less<>, class Proj = identity_fn>
void sort(Range&& range, Compare compare = {}, Proj proj = {})
{
    CPP_PIPELINES_CHECK_CONSTRAINTS(range, is_random_access_range);

    std::sort(std::begin(range), std::end(range), detail::invoke_binary{ std::ref(compare), std::ref(proj) });
}

template <class Range, class Compare = std::less<>, class Proj = identity_fn>
void stable_sort(Range&& range, Compare compare = {}, Proj proj = {})
{
    CPP_PIPELINES_CHECK_CONSTRAINTS(range, is_random_access_range);

    std::stable_sort(std::begin(range), std::end(range), detail::invoke_binary{ std::ref(compare), std::ref(proj) });
}

template <class Range, class OutputIter, class UnaryFunc, class Proj = identity_fn>
auto transform(Range&& range, OutputIter output, UnaryFunc func, Proj proj = {})
{
    CPP_PIPELINES_CHECK_CONSTRAINTS(range, is_input_range);

    return std::transform(std::begin(range), std::end(range), output, fn(std::ref(proj), std::ref(func)));
}

template <
    class Range1,
    class Range2,
    class OutputIter,
    class BinaryFunc,
    class Proj1 = identity_fn,
    class Proj2 = identity_fn,
    class = std::enable_if_t<is_output_iterator<OutputIter>::value>>
auto transform(Range1&& range1, Range2&& range2, OutputIter output, BinaryFunc func, Proj1 proj1 = {}, Proj2 proj2 = {})
{
    CPP_PIPELINES_CHECK_CONSTRAINTS(range1, is_input_range);
    CPP_PIPELINES_CHECK_CONSTRAINTS(range2, is_input_range);

    return std::transform(
        std::begin(range1),
        std::end(range1),
        std::begin(range2),
        output,
        detail::invoke_binary{ std::ref(func), std::ref(proj1), std::ref(proj2) });
}

template <class Range, class Output, class T, class BinaryFunc, class UnaryFunc, class Proj = identity_fn>
auto transform_exclusive_scan(Range&& range, Output output, T init, BinaryFunc func, UnaryFunc op, Proj proj = {})
{
    return std::transform_exclusive_scan(std::begin(range), std::end(range), output, init, func, fn(std::ref(proj), std::ref(op)));
}

template <class Range, class Output, class BinaryFunc, class UnaryFunc, class Proj = identity_fn>
auto transform_inclusive_scan(Range&& range, Output output, BinaryFunc func, UnaryFunc op, Proj proj = {})
{
    return std::transform_inclusive_scan(std::begin(range), std::end(range), output, std::ref(func), fn(std::ref(proj), std::ref(op)));
}

template <class Range, class T, class BinaryFunc, class UnaryFunc, class Proj = identity_fn>
auto transform_reduce(Range&& range, T init, BinaryFunc func, UnaryFunc op, Proj proj = {})
{
    return std::transform_reduce(std::begin(range), std::end(range), std::move(init), std::ref(func), fn(std::ref(proj), std::ref(op)));
}

template <class Policy = default_return_policy, class Range, class BinaryPred = std::equal_to<>, class Proj = identity_fn>
decltype(auto) unique(Range&& range, BinaryPred pred = {}, Proj proj = {})
{
    CPP_PIPELINES_CHECK_CONSTRAINTS(range, is_forward_range);

    return detail::invoke_algorithm<Policy>(std::begin(range), std::end(range), [&](auto b, auto e) {
        return std::unique(b, e, detail::invoke_binary{ std::ref(pred), std::ref(proj) });
    });
}

template <class Range, class OutputIter, class BinaryPred = std::equal_to<>, class Proj = identity_fn>
auto unique_copy(Range&& range, OutputIter output, BinaryPred pred = {}, Proj proj = {})
{
    CPP_PIPELINES_CHECK_CONSTRAINTS(range, is_forward_range);

    return std::unique_copy(
        std::begin(range), std::end(range), output, detail::invoke_binary{ std::ref(pred), std::ref(proj) });
}

template <class Policy = default_return_policy, class Range, class T, class Compare = std::less<>, class Proj = identity_fn>
decltype(auto) upper_bound(Range&& range, const T& value, Compare compare = {}, Proj proj = {})
{
    CPP_PIPELINES_CHECK_CONSTRAINTS(range, is_forward_range);

    return detail::invoke_algorithm<Policy>(std::begin(range), std::end(range), [&](auto b, auto e) {
        return detail::upper_bound(b, e, value, std::ref(compare), std::ref(proj));
    });
}

template <class Range, class Dest, class Proj = identity_fn>
auto overwrite(Range&& range, Dest&& dest, Proj proj = {})
{
    CPP_PIPELINES_CHECK_CONSTRAINTS(range, is_input_range);

    return detail::overwrite(std::begin(range), std::end(range), std::begin(dest), std::end(dest), std::ref(proj));
}

#undef CPP_PIPELINES_CHECK_CONSTRAINTS

}  // namespace cpp_pipelines::algorithm
