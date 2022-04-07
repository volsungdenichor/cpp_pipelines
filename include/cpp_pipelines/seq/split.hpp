#pragma once

#include <cpp_pipelines/algorithm.hpp>
#include <cpp_pipelines/iter_utils.hpp>
#include <cpp_pipelines/pipeline.hpp>
#include <cpp_pipelines/seq/views.hpp>
#include <cpp_pipelines/subrange.hpp>

// #include <cpp_pipelines/splitters.hpp>

namespace cpp_pipelines::seq
{
namespace detail
{
struct split_fn
{
    template <class Policy, class Range>
    struct view
    {
        Policy policy;
        Range range;

        constexpr view(Policy policy, Range range)
            : policy{ std::move(policy) }
            , range{ std::move(range) }
        {
        }

        struct iter
        {
            using inner_iterator = iterator_t<Range>;
            const view* parent;
            inner_iterator it;
            subrange<inner_iterator> current;

            constexpr iter() = default;

            constexpr iter(const view* parent, inner_iterator it)
                : parent{ parent }
                , it{ it }
            {
                update();
            }

            constexpr auto deref() const
            {
                return current;
            }

            constexpr void inc()
            {
                update();
            }

            constexpr bool is_equal(const iter& other) const
            {
                return current.begin() == other.current.begin();
            }

            constexpr void update()
            {
                const auto sub = subrange{ it, std::end(parent->range) };
                const auto separator = !sub.empty()
                                           ? parent->policy(sub)
                                           : subrange{ sub.end(), sub.end() };
                current = subrange{ it, separator.begin() };
                it = separator.end();
            }
        };

        constexpr auto begin() const
        {
            return iterator_interface{ iter{ this, std::begin(range) } };
        }

        constexpr auto end() const
        {
            return iterator_interface{ iter{ this, std::end(range) } };
        }
    };

    template <class Policy>
    struct impl
    {
        Policy policy;

        template <class Range>
        constexpr auto operator()(Range&& range) const
        {
            return view_interface{ view{ policy, all(std::forward<Range>(range)) } };
        }
    };

    template <class Policy>
    constexpr auto operator()(Policy policy) const
    {
        return make_pipeline(impl<Policy>{ std::move(policy) });
    }
};

static constexpr inline auto split = detail::split_fn{};

struct split_when_fn
{
    template <class Pred>
    struct impl
    {
        Pred pred;

        template <class Iter>
        constexpr subrange<Iter> operator()(subrange<Iter> sub) const
        {
            const auto it = algorithm::find_if(sub, pred);
            const auto n = advance(it, 1, std::end(sub));
            return subrange{ it, n };
        }
    };

    template <class Pred>
    constexpr auto operator()(Pred pred) const
    {
        return split(impl<Pred>{ std::move(pred) });
    }
};

struct split_on_element_fn
{
    template <class T>
    struct impl
    {
        T element;

        template <class Iter>
        constexpr subrange<Iter> operator()(subrange<Iter> sub) const
        {
            const auto it = algorithm::find(sub, element);
            const auto n = advance(it, 1, std::end(sub));
            return subrange{ it, n };
        }
    };

    template <class T>
    constexpr auto operator()(T element) const
    {
        return split(impl<T>{ std::move(element) });
    }
};

struct split_on_subrange_fn
{
    template <class Range>
    struct impl
    {
        Range r;

        template <class Iter>
        constexpr subrange<Iter> operator()(subrange<Iter> sub) const
        {
            const auto b = std::begin(sub);
            const auto e = std::end(sub);
            const auto it = algorithm::search(sub, r);
            const auto n = advance(it, r.size(), std::end(sub));
            return subrange{ it, n };
        }
    };

    template <class Range>
    constexpr auto operator()(Range&& subrange) const
    {
        auto r = all(std::forward<Range>(subrange));
        return split(impl<decltype(r)>{ std::move(r) });
    }
};

}  // namespace detail

static constexpr inline auto split = detail::split_fn{};
static constexpr inline auto split_when = detail::split_when_fn{};
static constexpr inline auto split_on_element = detail::split_on_element_fn{};
static constexpr inline auto split_on_subrange = detail::split_on_subrange_fn{};
}  // namespace cpp_pipelines::seq
