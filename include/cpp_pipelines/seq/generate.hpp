#pragma once

#include <cpp_pipelines/semiregular.hpp>
#include <cpp_pipelines/seq/views.hpp>
#include <limits>

namespace cpp_pipelines::seq
{
namespace detail
{
struct generate_fn
{
    template <class Func>
    struct view
    {
        Func func;

        constexpr view(Func func)
            : func{ std::move(func) }
        {
        }

        struct iter
        {
            semiregular<Func> func;
            using maybe_type = std::decay_t<decltype(std::invoke(func))>;
            mutable maybe_type current;
            std::ptrdiff_t index;

            constexpr iter()
                : func{}
                , current{}
                , index{ std::numeric_limits<std::ptrdiff_t>::max() }
            {
            }

            constexpr iter(Func func)
                : func{ std::move(func) }
                , current{ std::invoke(this->func) }
                , index{ 0 }
            {
            }

            constexpr decltype(auto) deref() const
            {
                return opt::get_value(current);
            }

            constexpr void inc()
            {
                current = std::invoke(func);
                ++index;
            }

            constexpr bool is_equal(const iter& other) const
            {
                return !opt::has_value(current) || index == other.index;
            }
        };

        using iterator = iterator_interface<iter>;

        constexpr iterator begin() const
        {
            return { func };
        }

        constexpr iterator end() const
        {
            return {};
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
