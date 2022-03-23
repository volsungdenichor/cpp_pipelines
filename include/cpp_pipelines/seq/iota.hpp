#pragma once

#include <cpp_pipelines/seq/views.hpp>

namespace cpp_pipelines::seq
{
namespace detail
{
struct iota_fn
{
    template <class T>
    struct view
    {
        T lo, up;

        struct iter
        {
            T value;

            constexpr iter() = default;

            constexpr iter(T value)
                : value{ value }
            {
            }

            constexpr T deref() const
            {
                return value;
            }

            constexpr void advance(std::ptrdiff_t offset)
            {
                value += offset;
            }

            constexpr std::ptrdiff_t distance_to(const iter& other) const
            {
                return other.value - value;
            }
        };

        constexpr auto begin() const
        {
            return iterator_interface{ iter{ lo } };
        }

        constexpr auto end() const
        {
            return iterator_interface{ iter{ up } };
        }
    };

    template <class T>
    constexpr auto operator()(T lo, type_identity_t<T> up) const
    {
        static_assert(std::is_integral_v<T>, "integral type required");
        return view_interface{ view<T>{ lo, up } };
    }

    template <class T>
    constexpr auto operator()(T lo) const
    {
        return (*this)(lo, std::numeric_limits<T>::max());
    }
};

}  // namespace detail

static constexpr inline auto iota = detail::iota_fn{};

}  // namespace cpp_pipelines::seq