#pragma once

#include <cpp_pipelines/opt.hpp>
#include <cpp_pipelines/pipeline.hpp>
#include <cpp_pipelines/seq/views.hpp>

namespace cpp_pipelines::seq
{
struct transform_maybe_fn
{
    template <class Func, class Range>
    struct view
    {
        Func func;
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
            inner_iterator it;
            using maybe_type = std::decay_t<decltype(invoke(parent->func, *it))>;
            maybe_type current;

            constexpr iter(const view* parent, inner_iterator it)
                : parent{ parent }
                , it{ it }
                , current{}
            {
                update();
            }

            constexpr decltype(auto) deref() const
            {
                return opt::get_value(current);
            }

            constexpr void inc()
            {
                ++it;
                update();
            }

            constexpr bool is_equal(const iter& other) const
            {
                return it == other.it;
            }

            constexpr void update()
            {
                while (it != std::end(parent->range))
                {
                    if (current = invoke(parent->func, *it); opt::has_value(current))
                    {
                        return;
                    }
                    ++it;
                }
            }
        };

        using iterator = iterator_interface<iter>;

        constexpr iterator begin() const
        {
            return { iter{ this, std::begin(range) } };
        }

        constexpr iterator end() const
        {
            return { iter{ this, std::end(range) } };
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

static constexpr inline auto transform_maybe = transform_maybe_fn{};

}  // namespace cpp_pipelines::seq
