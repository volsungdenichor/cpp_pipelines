#pragma once

#include <cpp_pipelines/pipeline.hpp>
#include <cpp_pipelines/seq/views.hpp>
#include <memory>

namespace cpp_pipelines::seq
{
struct join_fn
{
    template <class Range>
    struct view
    {
        Range range;

        constexpr view(Range range)
            : range{ std::move(range) }
        {
        }

        struct iter
        {
            using inner_iterator = iterator_t<Range>;
            const view* parent;
            inner_iterator it;
            using sub_type = std::decay_t<decltype(all(*it))>;
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
            auto end() const
            {
                return std::end(parent->range);
            }

            void update_sub()
            {
                sub = all(*it);
                sub_it = std::begin(sub);
            }

            void update()
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

    struct impl
    {
        template <class Range>
        constexpr auto operator()(Range&& range) const
        {
            return view_interface{ view{ all(std::forward<Range>(range)) } };
        }
    };

    constexpr auto operator()() const
    {
        return make_pipeline(impl{});
    }
};

static constexpr inline auto join = join_fn{};

}  // namespace cpp_pipelines::seq