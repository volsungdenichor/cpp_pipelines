#pragma once

#include <cpp_pipelines/seq/views.hpp>

namespace cpp_pipelines::seq
{
namespace detail
{
struct repeat_fn
{
    template <class T>
    struct view
    {
        T value;
        std::ptrdiff_t count;

        struct iter
        {
            const view* parent;
            std::ptrdiff_t index;

            constexpr iter() = default;

            constexpr iter(const view* parent, std::ptrdiff_t index)
                : parent{ parent }
                , index{ index }
            {
            }

            constexpr const T& deref() const
            {
                return parent->value;
            }

            constexpr void inc()
            {
                ++index;
            }

            constexpr bool is_equal(const iter& other) const
            {
                return index == other.index;
            }
        };

        using iterator = iterator_interface<iter>;

        constexpr iterator begin() const
        {
            return { this, 0 };
        }

        constexpr iterator end() const
        {
            return { this, count };
        }
    };

    template <class T>
    constexpr auto operator()(T value, std::ptrdiff_t count) const
    {
        return view_interface{ view<T>{ std::move(value), count } };
    }

    template <class T>
    constexpr auto operator()(T value) const
    {
        return (*this)(std::move(value), std::numeric_limits<std::ptrdiff_t>::max());
    }
};

struct single_fn
{
    template <class T>
    constexpr auto operator()(T value) const
    {
        return repeat_fn{}(std::move(value), 1);
    }
};

}  // namespace detail

static constexpr inline auto repeat = detail::repeat_fn{};
static constexpr inline auto single = detail::single_fn{};

}  // namespace cpp_pipelines::seq