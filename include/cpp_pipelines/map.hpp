#pragma once

#include <cpp_pipelines/functions.hpp>
#include <cpp_pipelines/seq/access.hpp>
#include <cpp_pipelines/seq/transform.hpp>
#include <cpp_pipelines/subrange.hpp>

namespace cpp_pipelines::map
{
namespace detail
{
struct equal_range_fn
{
    template <class K>
    struct impl
    {
        K key;

        template <class Map>
        constexpr auto operator()(Map& map) const
        {
            return subrange{ map.equal_range(key) } >>= seq::transform(get_value);
        }
    };

    template <class K>
    constexpr auto operator()(K key) const
    {
        return make_pipeline(impl<K>{ std::move(key) });
    }
};

struct maybe_at
{
    template <class K>
    constexpr auto operator()(K key) const
    {
        return equal_range_fn{}(std::move(key)) >>= seq::maybe_front;
    }
};

}  // namespace detail
constexpr inline auto equal_range = detail::equal_range_fn{};
constexpr inline auto maybe_at = detail::maybe_at{};
}  // namespace cpp_pipelines::map