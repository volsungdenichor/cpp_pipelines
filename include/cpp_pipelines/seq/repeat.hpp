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

        constexpr auto begin() const
        {
            return iterator_interface{ iter{ this, 0 } };
        }

        constexpr auto end() const
        {
            return iterator_interface{ iter{ this, count } };
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

}  // namespace detail

static constexpr inline auto repeat = detail::repeat_fn{};

}  // namespace cpp_pipelines::seq