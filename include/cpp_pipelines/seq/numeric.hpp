#pragma once

#include <cpp_pipelines/seq/transform.hpp>
#include <cpp_pipelines/seq/views.hpp>
#include <limits>

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

        using iterator = iterator_interface<iter>;

        constexpr iterator begin() const
        {
            return { lo };
        }

        constexpr iterator end() const
        {
            return { up };
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

struct range_fn
{
    template <class T>
    constexpr auto operator()(T lower, type_identity_t<T> upper) const
    {
        return iota_fn{}(lower, upper);
    }

    template <class T>
    constexpr auto operator()(T upper) const
    {
        return (*this)(T{}, upper);
    }
};

struct linspace_fn
{
    template <class T>
    constexpr auto operator()(T lower, type_identity_t<T> upper, std::ptrdiff_t count) const
    {
        static_assert(std::is_floating_point_v<T>, "linspace: floating point type expected");
        return iota_fn{}(0, count) |= transform([=](int n)
                                                { return lower + n * (upper - lower) / (count - 1); });
    }
};

}  // namespace detail

static constexpr inline auto iota = detail::iota_fn{};
static constexpr inline auto range = detail::range_fn{};
static constexpr inline auto linspace = detail::linspace_fn{};

}  // namespace cpp_pipelines::seq
