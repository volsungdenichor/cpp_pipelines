#pragma once

#include <cpp_pipelines/semiregular.hpp>
#include <cpp_pipelines/seq/views.hpp>

namespace cpp_pipelines::seq
{
namespace detail
{
struct generate_fn
{
    template <class Func>
    struct view
    {
        static constexpr auto sentinel = std::numeric_limits<std::ptrdiff_t>::max();
        mutable semiregular<Func> func;

        constexpr view(Func func)
            : func{ std::move(func) }
        {
        }

        struct iter
        {
            const view* parent;
            using maybe_type = std::decay_t<decltype(std::invoke(parent->func))>;
            mutable maybe_type current;
            std::ptrdiff_t index;

            constexpr iter() = default;

            constexpr iter(const view* parent, std::ptrdiff_t index)
                : parent{ parent }
                , index{ index }
            {
                if (index != sentinel)
                {
                    current = std::invoke(parent->func);
                }
            }

            constexpr decltype(auto) deref() const
            {
                return opt::get_value(current);
            }

            constexpr void inc()
            {
                current = std::invoke(parent->func);
                ++index;
            }

            constexpr bool is_equal(const iter& other) const
            {
                return !opt::has_value(current) || index == other.index;
            }
        };

        constexpr auto begin() const
        {
            return iterator_interface{ iter{ this, 0 } };
        }

        constexpr auto end() const
        {
            return iterator_interface{ iter{ this, sentinel } };
        }
    };

    template <class Func>
    constexpr auto operator()(Func func) const
    {
        return view_interface{ view{ std::move(func) } };
    }
};

struct generate_infinite_fn
{
    template <class Func>
    struct impl
    {
        mutable Func func;

        constexpr auto operator()() const
        {
            return std::optional{ invoke(func) };
        }
    };

    template <class Func>
    constexpr auto operator()(Func func) const
    {
        return generate_fn{}(impl<Func>{ std::move(func) });
    }
};

}  // namespace detail
static constexpr inline auto generate = detail::generate_fn{};
static constexpr inline auto generate_infinite = detail::generate_infinite_fn{};
}  // namespace cpp_pipelines::seq