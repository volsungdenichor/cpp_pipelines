#pragma once

#include <cpp_pipelines/pipeline.hpp>
#include <cpp_pipelines/seq/views.hpp>

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
            : policy{std::move(policy)}
            , range{std::move(range)}
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
                const auto sub = subrange{it, std::end(parent->range)};
                const auto separator = !sub.empty()
                    ? parent->policy(sub)
                    : subrange{sub.end(), sub.end()};
                current = subrange{it, separator.begin()};
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

}  // namespace detail

static constexpr inline auto split = detail::split_fn{};
}  // namespace cpp_pipelines::seq