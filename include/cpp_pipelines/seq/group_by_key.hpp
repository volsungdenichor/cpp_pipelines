#pragma once

#include <cpp_pipelines/pipeline.hpp>
#include <cpp_pipelines/seq/views.hpp>
#include <cpp_pipelines/subrange.hpp>

namespace cpp_pipelines::seq
{
namespace detail
{
struct group_by_key_fn
{
    template <class Func, class Range>
    struct view
    {
        semiregular<Func> func;
        Range range;

        constexpr view(Func func, Range range)
            : func{ std::move(func) }
            , range{ std::move(range) }
        {
        }

        struct iter
        {
            using inner_iterator = iterator_t<Range>;
            const view* parent;
            subrange<inner_iterator> current;

            constexpr iter() = default;

            constexpr iter(const view* parent, inner_iterator it)
                : parent{ parent }
                , current{ next(it) }
            {
            }

            constexpr auto deref() const
            {
                return current;
            }

            constexpr void inc()
            {
                current = next(current.end());
            }

            constexpr bool is_equal(const iter& other) const
            {
                return current.begin() == other.current.begin();
            }

        private:
            subrange<inner_iterator> next(inner_iterator it) const
            {
                inner_iterator prev = it;
                if (it != std::end(parent->range))
                {
                    using key_type = std::decay_t<decltype(invoke(parent->func, *it))>;
                    key_type key = invoke(parent->func, *it);
                    while (it != std::end(parent->range) && invoke(parent->func, *it) == key)
                    {
                        ++it;
                    }
                }
                return { prev, it };
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

    template <class Func>
    struct impl
    {
        Func func;

        template <class Range>
        constexpr auto operator()(Range&& range) const
        {
            return view_interface{ view{ func, all(std::forward<Range>(range)) } };
        }
    };

    template <class Func>
    constexpr auto operator()(Func func) const
    {
        return make_pipeline(impl<Func>{ std::move(func) });
    }
};

}  // namespace detail

static constexpr inline auto group_by_key = detail::group_by_key_fn{};
}  // namespace cpp_pipelines::seq