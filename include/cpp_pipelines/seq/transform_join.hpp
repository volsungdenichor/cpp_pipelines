#pragma once

#include <cpp_pipelines/pipeline.hpp>
#include <cpp_pipelines/seq/views.hpp>

namespace cpp_pipelines::seq
{
struct transform_join_fn
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
            using sub_type = std::decay_t<decltype(all(invoke(parent->func, *it)))>;
            using sub_iterator = iterator_t<sub_type>;
            sub_type sub;
            sub_iterator sub_it;

            constexpr iter(const view* parent, inner_iterator it)
                : parent{ parent }
                , it{ it }
                , sub{}
            {
                if (it != end())
                {
                    update_sub();
                    update();
                }
            }

            constexpr decltype(auto) deref() const
            {
                return to_return_type(*sub_it);
            }

            constexpr void inc()
            {
                if (++sub_it == std::end(sub))
                {
                    update();
                }
            }

            constexpr bool is_equal(const iter& other) const
            {
                return it == other.it;
            }

        private:
            constexpr auto end() const
            {
                return std::end(parent->range);
            }

            constexpr void update_sub()
            {
                sub = all(invoke(parent->func, *it));
                sub_it = std::begin(sub);
            }

            constexpr void update()
            {
                while (it != end() && sub_it == std::end(sub))
                {
                    if (++it != end())
                    {
                        update_sub();
                    }
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

static constexpr inline auto transform_join = transform_join_fn{};

}  // namespace cpp_pipelines::seq