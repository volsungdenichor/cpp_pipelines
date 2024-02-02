#pragma once

#include <cpp_pipelines/pipeline.hpp>
#include <cpp_pipelines/seq/views.hpp>

namespace cpp_pipelines::seq
{
namespace detail
{
struct intersperse_fn
{
    template <class Range, class T>
    struct view
    {
        Range range;
        T delimiter;

        constexpr view(Range range, T delimiter)
            : range{ std::move(range) }
            , delimiter{ std::move(delimiter) }
        {
        }

        struct iter
        {
            using inner_iterator = iterator_t<Range>;
            const view* parent;
            inner_iterator it;
            bool flag;

            constexpr iter() = default;

            constexpr iter(const view* parent, inner_iterator it)
                : parent{ parent }
                , it{ it }
                , flag{ true }
            {
            }

            constexpr decltype(auto) deref() const
            {
                return to_return_type(flag ? *it : parent->delimiter);
            }

            constexpr void inc()
            {
                if (flag)
                {
                    ++it;
                }
                flag = !flag;
            }

            constexpr bool is_equal(const iter& other) const
            {
                return it == other.it;
            }
        };

        using iterator = iterator_interface<iter>;

        constexpr iterator begin() const
        {
            return { this, std::begin(range) };
        }

        constexpr iterator end() const
        {
            return { this, std::end(range) };
        }
    };

    template <class T>
    struct impl
    {
        T delimiter;

        template <class Range>
        constexpr auto operator()(Range&& range) const
        {
            return view_interface{ view{ all(std::forward<Range>(range)), delimiter } };
        }
    };

    template <class T>
    constexpr auto operator()(T delimiter) const
    {
        return fn(impl<T>{ std::move(delimiter) });
    }
};

}  // namespace detail

static constexpr inline auto intersperse = detail::intersperse_fn{};
}  // namespace cpp_pipelines::seq